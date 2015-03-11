#include "Win32/BsVSCodeEditor.h"
#include <windows.h>
#include <atlbase.h>
#include "BsFileSystem.h"
#include "BsDataStream.h"

// Import EnvDTE
#import "libid:80cc9f66-e7d8-4ddd-85b6-d9e6cd0e93e2" version("8.0") lcid("0") raw_interfaces_only named_guids

namespace BansheeEngine
{
	LONG getRegistryStringValue(HKEY hKey, const WString& name, WString& value, const WString& defaultValue)
	{
		value = defaultValue;

		wchar_t strBuffer[512];
		DWORD strBufferSize = sizeof(strBuffer);
		ULONG result = RegQueryValueExW(hKey, name.c_str(), 0, nullptr, (LPBYTE)strBuffer, &strBufferSize);
		if (result == ERROR_SUCCESS)
			value = strBuffer;

		return result;
	}

	struct VSProjectInfo
	{
		WString GUID;
		WString name;
		Path path;
	};

	class VisualStudio
	{
	private:
		static const String SLN_TEMPLATE;
		static const String PROJ_ENTRY_TEMPLATE;
		static const String PROJ_PLATFORM_TEMPLATE;

		static const String PROJ_TEMPLATE;
		static const String REFERENCE_ENTRY_TEMPLATE;
		static const String REFERENCE_PROJECT_ENTRY_TEMPLATE;
		static const String REFERENCE_PATH_ENTRY_TEMPLATE;
		static const String CODE_ENTRY_TEMPLATE;
		static const String NON_CODE_ENTRY_TEMPLATE;

	public:
		static CComPtr<EnvDTE::_DTE> findRunningInstance(const CLSID& clsID, const Path& solutionPath)
		{
			CComPtr<IRunningObjectTable> runningObjectTable = nullptr;
			if (FAILED(GetRunningObjectTable(0, &runningObjectTable)))
				return nullptr;

			CComPtr<IEnumMoniker> enumMoniker = nullptr;
			if (FAILED(runningObjectTable->EnumRunning(&enumMoniker)))
				return nullptr;

			CComPtr<IMoniker> dteMoniker = nullptr;
			if (FAILED(CreateClassMoniker(clsID, &dteMoniker)))
				return nullptr;

			CComBSTR bstrSolution(solutionPath.toWString(Path::PathType::Windows).c_str());
			CComPtr<IMoniker> moniker;
			ULONG count = 0;
			while (enumMoniker->Next(1, &moniker, &count) == S_OK)
			{
				if (moniker->IsEqual(dteMoniker))
				{
					CComPtr<IUnknown> curObject = nullptr;
					HRESULT result = runningObjectTable->GetObject(moniker, &curObject);
					moniker = nullptr;

					if (result != S_OK)
						continue;

					CComPtr<EnvDTE::_DTE> dte;
					curObject->QueryInterface(__uuidof(EnvDTE::_DTE), (void**)&dte);

					if (dte == nullptr)
						return nullptr;

					CComPtr<EnvDTE::_Solution> solution;
					if (FAILED(dte->get_Solution(&solution)))
						continue;

					CComBSTR fullName;
					if (FAILED(solution->get_FullName(&fullName)))
						continue;

					if (fullName == bstrSolution)
						return dte;
				}
			}

			return nullptr;
		}

		static CComPtr<EnvDTE::_DTE> openInstance(const CLSID& clsid, const Path& solutionPath)
		{
			CComPtr<IUnknown> newInstance = nullptr;
			if (FAILED(::CoCreateInstance(clsid, nullptr, CLSCTX_LOCAL_SERVER, EnvDTE::IID__DTE, (LPVOID*)&newInstance)))
				return nullptr;

			CComPtr<EnvDTE::_DTE> dte;
			newInstance->QueryInterface(__uuidof(EnvDTE::_DTE), (void**)&dte);

			if (dte == nullptr)
				return nullptr;

			dte->put_UserControl(TRUE);

			CComPtr<EnvDTE::_Solution> solution;
			if (FAILED(dte->get_Solution(&solution)))
				return nullptr;

			CComBSTR bstrSolution(solutionPath.toWString(Path::PathType::Windows).c_str());
			if (FAILED(solution->Open(bstrSolution)))
				return nullptr;

			// Wait until VS opens
			UINT32 elapsed = 0;
			while (elapsed < 10000)
			{
				EnvDTE::Window* window = nullptr;
				if (SUCCEEDED(dte->get_MainWindow(&window)))
					return dte;

				Sleep(100);
				elapsed += 100;
			}

			return nullptr;
		}

		static bool openFile(CComPtr<EnvDTE::_DTE> dte, const Path& filePath, UINT32 line)
		{
			// Open file
			CComPtr<EnvDTE::ItemOperations> itemOperations;
			if (FAILED(dte->get_ItemOperations(&itemOperations)))
				return false;

			CComBSTR bstrFilePath(filePath.toWString(Path::PathType::Windows).c_str());
			CComBSTR bstrKind(EnvDTE::vsViewKindPrimary);
			CComPtr<EnvDTE::Window> window = nullptr;
			if (FAILED(itemOperations->OpenFile(bstrFilePath, bstrKind, &window)))
				return false;

			// Scroll to line
			CComPtr<EnvDTE::Document> activeDocument;
			if (SUCCEEDED(dte->get_ActiveDocument(&activeDocument)))
			{
				CComPtr<IDispatch> selection;
				if (SUCCEEDED(activeDocument->get_Selection(&selection)))
				{
					CComPtr<EnvDTE::TextSelection> textSelection;
					if (SUCCEEDED(selection->QueryInterface(&textSelection)))
					{
						textSelection->GotoLine(line, TRUE);
					}
				}
			}

			// Bring the window in focus
			window = nullptr;
			if (SUCCEEDED(dte->get_MainWindow(&window)))
			{
				window->Activate();

				HWND hWnd;
				window->get_HWnd((LONG*)&hWnd);
				SetForegroundWindow(hWnd);
			}

			return true;
		}

		static String getProjectGUID(const WString& projectName)
		{
			static const String guidTemplate = "{0}-{1}-{2}-{3}-{4}";
			String hash = md5(projectName);

			String output = StringUtil::format(guidTemplate, hash.substr(0, 8),
				hash.substr(8, 4), hash.substr(12, 4), hash.substr(16, 4), hash.substr(20, 12));
			StringUtil::toUpperCase(output);

			return output;
		}

		static String writeSolution(VisualStudioVersion version, const CodeSolutionData& data)
		{
			struct VersionData
			{
				String formatVersion;
			};

			Map<VisualStudioVersion, VersionData> versionData =
			{
				{ VisualStudioVersion::VS2008, { "10.00" } },
				{ VisualStudioVersion::VS2010, { "11.00" } },
				{ VisualStudioVersion::VS2012, { "12.00" } },
				{ VisualStudioVersion::VS2013, { "12.00" } },
				{ VisualStudioVersion::VS2015, { "12.00" } }
			};

			StringStream projectEntriesStream;
			StringStream projectPlatformsStream;
			for (auto& project : data.projects)
			{
				String guid = getProjectGUID(project.name);
				String projectName = toString(project.name);

				projectEntriesStream << StringUtil::format(PROJ_ENTRY_TEMPLATE, projectName, projectName + ".csproj", guid);
				projectPlatformsStream << StringUtil::format(PROJ_PLATFORM_TEMPLATE, guid);
			}

			String projectEntries = projectEntriesStream.str();
			String projectPlatforms = projectPlatformsStream.str();

			return StringUtil::format(SLN_TEMPLATE, versionData[version].formatVersion, projectEntries, projectPlatforms);
		}

		static String writeProject(VisualStudioVersion version, const CodeProjectData& projectData)
		{
			struct VersionData
			{
				String toolsVersion;
			};

			Map<VisualStudioVersion, VersionData> versionData =
			{
				{ VisualStudioVersion::VS2008, { "3.5" } },
				{ VisualStudioVersion::VS2010, { "4.0" } },
				{ VisualStudioVersion::VS2012, { "4.0" } },
				{ VisualStudioVersion::VS2013, { "12.0" } },
				{ VisualStudioVersion::VS2015, { "13.0" } }
			};

			StringStream tempStream;
			for (auto& codeEntry : projectData.codeFiles)
				tempStream << StringUtil::format(CODE_ENTRY_TEMPLATE, codeEntry.toString());

			String codeEntries = tempStream.str();
			tempStream.str("");
			tempStream.clear();

			for (auto& nonCodeEntry : projectData.nonCodeFiles)
				tempStream << StringUtil::format(NON_CODE_ENTRY_TEMPLATE, nonCodeEntry.toString());

			String nonCodeEntries = tempStream.str();
			tempStream.str("");
			tempStream.clear();

			for (auto& referenceEntry : projectData.assemblyReferences)
			{
				String referenceName = toString(referenceEntry.name);

				if (referenceEntry.path.isEmpty())
					tempStream << StringUtil::format(REFERENCE_ENTRY_TEMPLATE, referenceName);
				else
					tempStream << StringUtil::format(REFERENCE_PATH_ENTRY_TEMPLATE, referenceName, referenceEntry.path.toString());
			}

			String referenceEntries = tempStream.str();
			tempStream.str("");
			tempStream.clear();

			for (auto& referenceEntry : projectData.projectReferences)
			{
				String referenceName = toString(referenceEntry.name);
				String projectGUID = getProjectGUID(referenceEntry.name);

				tempStream << StringUtil::format(REFERENCE_PROJECT_ENTRY_TEMPLATE, referenceName, projectGUID);
			}

			String projectReferenceEntries = tempStream.str();
			tempStream.str("");
			tempStream.clear();

			for (auto& define : projectData.defines)
				tempStream << toString(define) << ";";

			String defines = tempStream.str();
			String projectGUID = getProjectGUID(projectData.name);

			return StringUtil::format(PROJ_TEMPLATE, versionData[version].toolsVersion, projectGUID, 
				toString(projectData.name), defines, referenceEntries, projectReferenceEntries, codeEntries, nonCodeEntries);
		}
	};

	const String VisualStudio::SLN_TEMPLATE =
		R"(Microsoft Visual Studio Solution File, Format Version {0}
# Visual Studio 2013
VisualStudioVersion = 12.0.30723.0
MinimumVisualStudioVersion = 10.0.40219.1{1}
Global
	GlobalSection(SolutionConfigurationPlatforms) = preSolution
		Debug|Any CPU = Debug|Any CPU
		Release|Any CPU = Release|Any CPU
	EndGlobalSection
	GlobalSection(ProjectConfigurationPlatforms) = postSolution{2}
	EndGlobalSection
	GlobalSection(SolutionProperties) = preSolution
		HideSolutionNode = FALSE
	EndGlobalSection
EndGlobal
)";

	const String VisualStudio::PROJ_ENTRY_TEMPLATE =
		R"(
Project("\{FAE04EC0-301F-11D3-BF4B-00C04F79EFBC\}") = "{0}", "{1}", "\{{2}\}"
EndProject)";

	const String VisualStudio::PROJ_PLATFORM_TEMPLATE =
		R"(
		\{{0}\}.Debug|Any CPU.ActiveCfg = Debug|Any CPU
		\{{0}\}.Debug|Any CPU.Build.0 = Debug|Any CPU
		\{{0}\}.Release|Any CPU.ActiveCfg = Release|Any CPU
		\{{0}\}.Release|Any CPU.Build.0 = Release|Any CPU)";

	const String VisualStudio::PROJ_TEMPLATE =
		R"literal(<?xml version="1.0" encoding="utf-8"?>
<Project ToolsVersion="{0}" DefaultTargets="Build" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <Import Project="$(MSBuildExtensionsPath)\\$(MSBuildToolsVersion)\\Microsoft.Common.props" Condition="Exists('$(MSBuildExtensionsPath)\\$(MSBuildToolsVersion)\\Microsoft.Common.props')" />
  <PropertyGroup>
    <Configuration Condition = " '$(Configuration)' == '' ">Debug</Configuration>
    <Platform Condition = " '$(Platform)' == '' ">AnyCPU</Platform>
    <ProjectGuid>\{{1}\}</ProjectGuid>
    <OutputType>Library</OutputType>
    <AppDesignerFolder>Properties</AppDesignerFolder>
    <RootNamespace></RootNamespace>
    <AssemblyName>{2}</AssemblyName>
    <TargetFrameworkVersion>v4.0</TargetFrameworkVersion>
    <FileAlignment>512</FileAlignment>
    <BaseDirectory>Resources</BaseDirectory>
    <SchemaVersion>2.0</SchemaVersion>
  </PropertyGroup>
  <PropertyGroup Condition = " '$(Configuration)|$(Platform)' == 'Debug|AnyCPU' ">
    <DebugSymbols>true</DebugSymbols>
    <DebugType>full</DebugType>
    <Optimize>false</Optimize>
    <OutputPath>Internal\\Temp\\Assemblies\\Debug\\</OutputPath>
    <BaseIntermediateOutputPath>Internal\\Temp\\Assemblies\\</BaseIntermediateOutputPath>
    <DefineConstants>DEBUG;TRACE;{3}</DefineConstants>
    <ErrorReport>prompt</ErrorReport>
    <WarningLevel>4</WarningLevel >
  </PropertyGroup>
  <PropertyGroup Condition = " '$(Configuration)|$(Platform)' == 'Release|AnyCPU' ">
    <DebugType>pdbonly</DebugType>
    <Optimize>true</Optimize>
    <OutputPath>Internal\\Temp\\Assemblies\\Release\\</OutputPath>
	<BaseIntermediateOutputPath>Internal\\Temp\\Assemblies\\</BaseIntermediateOutputPath>
    <DefineConstants>TRACE;{3}</DefineConstants>
    <ErrorReport>prompt</ErrorReport>
    <WarningLevel>4</WarningLevel>
  </PropertyGroup>
  <ItemGroup>{4}
  </ItemGroup>
  <ItemGroup>{5}
  </ItemGroup>
  <ItemGroup>{6}
  </ItemGroup>
  <ItemGroup>{7}
  </ItemGroup>
  <Import Project = "$(MSBuildToolsPath)\\Microsoft.CSharp.targets"/>
</Project>)literal";

	const String VisualStudio::REFERENCE_ENTRY_TEMPLATE =
		R"(
    <Reference Include="{0}"/>)";

	const String VisualStudio::REFERENCE_PATH_ENTRY_TEMPLATE =
		R"(
    <Reference Include="{0}">
      <HintPath>{1}</HintPath>
    </Reference>)";

	const String VisualStudio::REFERENCE_PROJECT_ENTRY_TEMPLATE =
		R"(
    <ProjectReference Include="{0}.csproj">
      <Project>\{{1}\}</Project>
      <Name>{0}</Name>
    </ProjectReference>)";

	const String VisualStudio::CODE_ENTRY_TEMPLATE =
		R"(
    <Compile Include="{0}"/>)";

	const String VisualStudio::NON_CODE_ENTRY_TEMPLATE =
		R"(
    <None Include="{0}"/>)";

	VSCodeEditor::VSCodeEditor(VisualStudioVersion version, const Path& execPath, const WString& CLSID)
		:mCLSID(CLSID), mExecPath(execPath), mVersion(version)
	{
		
	}

	void VSCodeEditor::openFile(const Path& solutionPath, const Path& filePath, UINT32 lineNumber) const
	{
		CLSID clsID;
		if (FAILED(CLSIDFromString(mCLSID.c_str(), &clsID)))
			return;

		CComPtr<EnvDTE::_DTE> dte = VisualStudio::findRunningInstance(clsID, solutionPath);
		if (dte == nullptr)
			dte = VisualStudio::openInstance(clsID, solutionPath);

		if (dte == nullptr)
			return;

		VisualStudio::openFile(dte, filePath, lineNumber);
	}

	void VSCodeEditor::syncSolution(const CodeSolutionData& data, const Path& outputPath) const
	{
		String solutionString = VisualStudio::writeSolution(mVersion, data);
		solutionString = StringUtil::replaceAll(solutionString, "\n", "\r\n");
		Path solutionPath = outputPath;
		solutionPath.append(data.name + L".sln");

		for (auto& project : data.projects)
		{
			String projectString = VisualStudio::writeProject(mVersion, project);
			projectString = StringUtil::replaceAll(projectString, "\n", "\r\n");

			Path projectPath = outputPath;
			projectPath.append(project.name + L".csproj");

			DataStreamPtr projectStream = FileSystem::createAndOpenFile(projectPath);
			projectStream->write(projectString.c_str(), projectString.size() * sizeof(String::value_type));
			projectStream->close();
		}

		DataStreamPtr solutionStream = FileSystem::createAndOpenFile(solutionPath);
		solutionStream->write(solutionString.c_str(), solutionString.size() * sizeof(String::value_type));
		solutionStream->close();
	}

	VSCodeEditorFactory::VSCodeEditorFactory()
		:mAvailableVersions(getAvailableVersions())
	{ 
		for (auto& version : mAvailableVersions)
			mAvailableEditors.push_back(version.first);
	}

	Map<CodeEditorType, VSCodeEditorFactory::VSVersionInfo> VSCodeEditorFactory::getAvailableVersions() const
	{
#if BS_ARCH_TYPE == BS_ARCHITECTURE_x86_64
		bool is64bit = true;
#else
		bool is64bit = false;
		IsWow64Process(GetCurrentProcess(), &is64bit);
#endif

		WString registryKeyRoot;
		if (is64bit)
			registryKeyRoot = L"SOFTWARE\\Wow6432Node\\Microsoft"; 
		else
			registryKeyRoot = L"SOFTWARE\\Microsoft";

		struct VersionData
		{
			CodeEditorType type;
			WString registryKey;
			WString name;
			WString executable;
		};

		Map<VisualStudioVersion, VersionData> versionToVersionNumber =
		{ 
			{ VisualStudioVersion::VS2008, { CodeEditorType::VS2008, L"VisualStudio\\9.0", L"Visual Studio 2008", L"devenv.exe" } },
			{ VisualStudioVersion::VS2010, { CodeEditorType::VS2010, L"VisualStudio\\10.0", L"Visual Studio 2010", L"devenv.exe" } },
			{ VisualStudioVersion::VS2012, { CodeEditorType::VS2012, L"VisualStudio\\11.0", L"Visual Studio 2012", L"devenv.exe" } },
			{ VisualStudioVersion::VS2013, { CodeEditorType::VS2013, L"VisualStudio\\12.0", L"Visual Studio 2013", L"devenv.exe" } },
			{ VisualStudioVersion::VS2015, { CodeEditorType::VS2015, L"VisualStudio\\13.0", L"Visual Studio 2015", L"devenv.exe" } }
		};

		Map<CodeEditorType, VSVersionInfo> versionInfo;
		for(auto version : versionToVersionNumber)
		{
			WString registryKey = registryKeyRoot + L"\\" + version.second.registryKey;

			HKEY regKey;
			LONG result = RegOpenKeyExW(HKEY_LOCAL_MACHINE, registryKey.c_str(), 0, KEY_READ, &regKey);
			if (result != ERROR_SUCCESS)
				continue;

			WString installPath;
			getRegistryStringValue(regKey, L"InstallDir", installPath, StringUtil::WBLANK);
			if (installPath.empty())
				continue;

			WString clsID;
			getRegistryStringValue(regKey, L"ThisVersionDTECLSID", clsID, StringUtil::WBLANK);

			VSVersionInfo info;
			info.name = version.second.name;
			info.execPath = installPath.append(version.second.executable);
			info.CLSID = clsID;
			info.version = version.first;

			versionInfo[version.second.type] = info;
		}

		// TODO - Also query for VSExpress and VSCommunity (their registry keys are different)

		return versionInfo;
	}

	CodeEditor* VSCodeEditorFactory::create(CodeEditorType type) const
	{
		auto findIter = mAvailableVersions.find(type);
		if (findIter == mAvailableVersions.end())
			return nullptr;

		// TODO - Also create VSExpress and VSCommunity editors

		return bs_new<VSCodeEditor>(findIter->second.version, findIter->second.execPath, findIter->second.CLSID);
	}
}