#include "BsScriptEditorApplication.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"
#include "BsMonoUtil.h"
#include "BsEditorApplication.h"
#include "BsProjectLibrary.h"
#include "BsProjectResourceMeta.h"
#include "BsPrefab.h"
#include "BsPrefabUtility.h"
#include "BsSceneManager.h"
#include "BsEditorWindowManager.h"
#include "BsMainEditorWindow.h"
#include "BsGUIStatusBar.h"
#include "BsScriptEditorTestSuite.h"
#include "BsTestOutput.h"
#include "BsScriptManager.h"
#include "BsPlatform.h"
#include "BsResources.h"
#include "BsScriptEditorWindow.h"
#include "BsEditorWindowBase.h"
#include "BsProfilerOverlay.h"
#include "BsCGUIWidget.h"
#include "BsSceneObject.h"
#include "BsCCamera.h"
#include <BsScriptInput.h>

namespace BansheeEngine
{
	bool ScriptEditorApplication::mRequestProjectLoad = false;
	bool ScriptEditorApplication::mRequestAssemblyReload = false;
	Path ScriptEditorApplication::mProjectLoadPath;
	HEvent ScriptEditorApplication::OnStatusBarClickedConn;

	ScriptEditorApplication::OnProjectLoadedThunkDef ScriptEditorApplication::onProjectLoadedThunk;
	ScriptEditorApplication::OnStatusBarClickedThunkDef ScriptEditorApplication::onStatusBarClickedThunk;

	ScriptEditorApplication::ScriptEditorApplication(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptEditorApplication::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_SetStatusScene", &ScriptEditorApplication::internal_SetStatusScene);
		metaData.scriptClass->addInternalCall("Internal_SetStatusProject", &ScriptEditorApplication::internal_SetStatusProject);
		metaData.scriptClass->addInternalCall("Internal_SetStatusCompiling", &ScriptEditorApplication::internal_SetStatusCompiling);
		metaData.scriptClass->addInternalCall("Internal_GetProjectPath", &ScriptEditorApplication::internal_GetProjectPath);
		metaData.scriptClass->addInternalCall("Internal_GetProjectName", &ScriptEditorApplication::internal_GetProjectName);
		metaData.scriptClass->addInternalCall("Internal_GetProjectLoaded", &ScriptEditorApplication::internal_GetProjectLoaded);
		metaData.scriptClass->addInternalCall("Internal_GetCompilerPath", &ScriptEditorApplication::internal_GetCompilerPath);
		metaData.scriptClass->addInternalCall("Internal_GetBuiltinAssemblyPath", &ScriptEditorApplication::internal_GetBuiltinAssemblyPath);
		metaData.scriptClass->addInternalCall("Internal_GetScriptAssemblyPath", &ScriptEditorApplication::internal_GetScriptAssemblyPath);
		metaData.scriptClass->addInternalCall("Internal_GetFrameworkAssemblyPath", &ScriptEditorApplication::internal_GetFrameworkAssemblyPath);
		metaData.scriptClass->addInternalCall("Internal_GetEngineAssemblyName", &ScriptEditorApplication::internal_GetEngineAssemblyName);
		metaData.scriptClass->addInternalCall("Internal_GetEditorAssemblyName", &ScriptEditorApplication::internal_GetEditorAssemblyName);
		metaData.scriptClass->addInternalCall("Internal_GetScriptGameAssemblyName", &ScriptEditorApplication::internal_GetScriptGameAssemblyName);
		metaData.scriptClass->addInternalCall("Internal_GetScriptEditorAssemblyName", &ScriptEditorApplication::internal_GetScriptEditorAssemblyName);
		metaData.scriptClass->addInternalCall("Internal_SaveScene", &ScriptEditorApplication::internal_SaveScene);
		metaData.scriptClass->addInternalCall("Internal_IsValidProject", &ScriptEditorApplication::internal_IsValidProject);
		metaData.scriptClass->addInternalCall("Internal_SaveProject", &ScriptEditorApplication::internal_SaveProject);
		metaData.scriptClass->addInternalCall("Internal_LoadProject", &ScriptEditorApplication::internal_LoadProject);
		metaData.scriptClass->addInternalCall("Internal_UnloadProject", &ScriptEditorApplication::internal_UnloadProject);
		metaData.scriptClass->addInternalCall("Internal_CreateProject", &ScriptEditorApplication::internal_CreateProject);
		metaData.scriptClass->addInternalCall("Internal_ReloadAssemblies", &ScriptEditorApplication::internal_ReloadAssemblies);
		metaData.scriptClass->addInternalCall("Internal_OpenExternally", &ScriptEditorApplication::internal_OpenExternally);
		metaData.scriptClass->addInternalCall("Internal_RunUnitTests", &ScriptEditorApplication::internal_RunUnitTests);
		metaData.scriptClass->addInternalCall("Internal_Quit", &ScriptEditorApplication::internal_Quit);

		onProjectLoadedThunk = (OnProjectLoadedThunkDef)metaData.scriptClass->getMethod("Internal_OnProjectLoaded")->getThunk();
		onStatusBarClickedThunk = (OnStatusBarClickedThunkDef)metaData.scriptClass->getMethod("Internal_OnStatusBarClicked")->getThunk();
	}

	void ScriptEditorApplication::startUp()
	{
		MainEditorWindow* mainWindow = EditorWindowManager::instance().getMainWindow();
		OnStatusBarClickedConn = mainWindow->getStatusBar().onMessageClicked.connect(&ScriptEditorApplication::onStatusBarClicked);
	}

	void ScriptEditorApplication::shutDown()
	{
		OnStatusBarClickedConn.disconnect();
	}

	void ScriptEditorApplication::update()
	{
		// Project load must be delayed when requested from managed code because it
		// triggers managed assembly reload, and that can't be performed when called
		// from the Mono thread.
		if (mRequestProjectLoad)
		{
			gEditorApplication().loadProject(mProjectLoadPath);

			mRequestProjectLoad = false;
			mRequestAssemblyReload = false;
			MonoUtil::invokeThunk(onProjectLoadedThunk);
		}
		else if (mRequestAssemblyReload)
		{
			ScriptManager::instance().reload();
			mRequestAssemblyReload = false;
		}
	}

	void ScriptEditorApplication::onStatusBarClicked()
	{
		MonoUtil::invokeThunk(onStatusBarClickedThunk);
	}

	void ScriptEditorApplication::internal_SetStatusScene(MonoString* name, bool modified)
	{
		WString nativeScene = MonoUtil::monoToWString(name);

		MainEditorWindow* mainWindow = EditorWindowManager::instance().getMainWindow();
		mainWindow->getStatusBar().setScene(nativeScene, modified);
	}

	void ScriptEditorApplication::internal_SetStatusProject(bool modified)
	{
		MainEditorWindow* mainWindow = EditorWindowManager::instance().getMainWindow();

		if (gEditorApplication().isProjectLoaded())
			mainWindow->getStatusBar().setProject(gEditorApplication().getProjectName(), modified);
		else
			mainWindow->getStatusBar().setProject(L"None", false);
	}

	void ScriptEditorApplication::internal_SetStatusCompiling(bool compiling)
	{
		MainEditorWindow* mainWindow = EditorWindowManager::instance().getMainWindow();
		mainWindow->getStatusBar().setIsCompiling(compiling);
	}

	MonoString* ScriptEditorApplication::internal_GetProjectPath()
	{
		Path projectPath = gEditorApplication().getProjectPath();

		return MonoUtil::wstringToMono(MonoManager::instance().getDomain(), projectPath.toWString());
	}

	MonoString* ScriptEditorApplication::internal_GetProjectName()
	{
		WString projectName = gEditorApplication().getProjectName();

		return MonoUtil::wstringToMono(MonoManager::instance().getDomain(), projectName);
	}

	bool ScriptEditorApplication::internal_GetProjectLoaded()
	{
		return gEditorApplication().isProjectLoaded();
	}

	MonoString* ScriptEditorApplication::internal_GetCompilerPath()
	{
		Path compilerPath = MonoManager::instance().getCompilerPath();

		return MonoUtil::wstringToMono(MonoManager::instance().getDomain(), compilerPath.toWString());
	}

	MonoString* ScriptEditorApplication::internal_GetBuiltinAssemblyPath()
	{
		Path assemblyFolder = gEditorApplication().getBuiltinAssemblyFolder();

		return MonoUtil::wstringToMono(MonoManager::instance().getDomain(), assemblyFolder.toWString());
	}

	MonoString* ScriptEditorApplication::internal_GetScriptAssemblyPath()
	{
		Path assemblyFolder = gEditorApplication().getScriptAssemblyFolder();

		return MonoUtil::wstringToMono(MonoManager::instance().getDomain(), assemblyFolder.toWString());
	}

	MonoString* ScriptEditorApplication::internal_GetFrameworkAssemblyPath()
	{
		Path assemblyFolder = MonoManager::instance().getFrameworkAssembliesFolder();

		return MonoUtil::wstringToMono(MonoManager::instance().getDomain(), assemblyFolder.toWString());
	}

	MonoString* ScriptEditorApplication::internal_GetEngineAssemblyName()
	{
		return MonoUtil::wstringToMono(MonoManager::instance().getDomain(), toWString(ENGINE_ASSEMBLY) + L".dll");
	}

	MonoString* ScriptEditorApplication::internal_GetEditorAssemblyName()
	{
		return MonoUtil::wstringToMono(MonoManager::instance().getDomain(), toWString(EDITOR_ASSEMBLY) + L".dll");
	}

	MonoString* ScriptEditorApplication::internal_GetScriptGameAssemblyName()
	{
		return MonoUtil::wstringToMono(MonoManager::instance().getDomain(), toWString(SCRIPT_GAME_ASSEMBLY) + L".dll");
	}

	MonoString* ScriptEditorApplication::internal_GetScriptEditorAssemblyName()
	{
		return MonoUtil::wstringToMono(MonoManager::instance().getDomain(), toWString(SCRIPT_EDITOR_ASSEMBLY) + L".dll");
	}

	MonoString* ScriptEditorApplication::internal_SaveScene(MonoString* path)
	{
		Path nativePath = MonoUtil::monoToWString(path);
		HSceneObject sceneRoot = gSceneManager().getRootNode();
		
		ProjectLibrary::LibraryEntry* entry = gProjectLibrary().findEntry(nativePath);
		HPrefab scene;
		if (entry != nullptr)
		{
			if (entry->type == ProjectLibrary::LibraryEntryType::Directory)
				return nullptr;

			ProjectLibrary::ResourceEntry* resEntry = static_cast<ProjectLibrary::ResourceEntry*>(entry);
			if (resEntry->meta == nullptr || resEntry->meta->getTypeID() != TID_Prefab)
				return nullptr;

			scene = static_resource_cast<Prefab>(gProjectLibrary().load(nativePath));
			scene->update(sceneRoot);

			gProjectLibrary().saveEntry(scene);
		}
		else
		{
			scene = Prefab::create(sceneRoot);
			gProjectLibrary().createEntry(scene, nativePath);
		}

		return MonoUtil::stringToMono(MonoManager::instance().getDomain(), scene.getUUID());
	}

	bool ScriptEditorApplication::internal_IsValidProject(MonoString* path)
	{
		Path nativePath = MonoUtil::monoToWString(path);
		return gEditorApplication().isValidProjectPath(nativePath);
	}

	void ScriptEditorApplication::internal_SaveProject()
	{
		gEditorApplication().saveProject();
	}

	void ScriptEditorApplication::internal_LoadProject(MonoString* path)
	{
		mRequestProjectLoad = true;
		mProjectLoadPath = MonoUtil::monoToWString(path);
	}

	void ScriptEditorApplication::internal_UnloadProject()
	{
		gEditorApplication().unloadProject();
	}

	void ScriptEditorApplication::internal_CreateProject(MonoString* path)
	{
		Path nativePath = MonoUtil::monoToWString(path);

		gEditorApplication().createProject(nativePath);
	}

	void ScriptEditorApplication::internal_ReloadAssemblies()
	{
		mRequestAssemblyReload = true;
	}

	void ScriptEditorApplication::internal_OpenExternally(MonoString* path)
	{
		Path nativePath = MonoUtil::monoToWString(path);

		PlatformUtility::open(nativePath);
	}

	void ScriptEditorApplication::internal_RunUnitTests()
	{
#if BS_DEBUG_MODE
		TestSuitePtr testSuite = TestSuite::create<ScriptEditorTestSuite>();
		ExceptionTestOutput testOutput;
		testSuite->run(testOutput);
#endif
	}

	void ScriptEditorApplication::internal_Quit()
	{
		gApplication().stopMainLoop();
	}
}
