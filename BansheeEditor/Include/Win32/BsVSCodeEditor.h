#pragma once

#include "BsEditorPrerequisites.h"
#include "BsCodeEditor.h"

namespace BansheeEngine
{
	/**
	 * @brief	Recognized types of Microsoft Visual Studio versions
	 *			recognized by VSCodeEditor.
	 */
	enum class VisualStudioVersion
	{
		VS2008,
		VS2010,
		VS2012,
		VS2013,
		VS2015
	};

	/**
	 * @brief	Code editor implementation that handles interacting with Microsoft Visual Studio.
	 */
	class BS_ED_EXPORT VSCodeEditor : public CodeEditor
	{
	public:
		VSCodeEditor(VisualStudioVersion version, const Path& execPath, const WString& CLSID);

		/**
		 * @copydoc	CodeEditor::openFile
		 */
		void openFile(const Path& solutionPath, const Path& filePath, UINT32 lineNumber) const override;

		/**
		 * @copydoc	CodeEditor::syncSolution
		 */
		void syncSolution(const CodeSolutionData& data, const Path& outputPath) const override;

	private:
		VisualStudioVersion mVersion;
		Path mExecPath;
		WString mCLSID;
	};

	/**
	 * @brief	Code editor factory used for creating specific instances 
	 *			of the Visual Studio code editor object.
	 */
	class BS_ED_EXPORT VSCodeEditorFactory : public CodeEditorFactory
	{
	public:
		VSCodeEditorFactory();

		/**
		 * @copydoc	CodeEditorFactory::getAvailableEditors
		 */
		const Vector<CodeEditorType>& getAvailableEditors() const override { return mAvailableEditors; }

		/**
		 * @copydoc	CodeEditorFactory::create
		 */
		CodeEditor* create(CodeEditorType editor) const override;

	private:
		/**
		 * @brief	Contains detailed information about a specific Visual Studio version.
		 */
		struct VSVersionInfo
		{
			WString name;
			Path execPath;
			WString CLSID;
			VisualStudioVersion version;
		};

		/**
		 * @brief	Returns a list of Visual Studio versions installed on this machine.
		 */
		Map<CodeEditorType, VSVersionInfo> getAvailableVersions() const;

		Map<CodeEditorType, VSVersionInfo> mAvailableVersions;
		Vector<CodeEditorType> mAvailableEditors;
	};
}