#pragma once

#include "BsScriptEditorPrerequisites.h"
#include "BsScriptObject.h"
#include "BsBuiltinEditorResources.h"

namespace BansheeEngine
{
	/**
	 * @brief	Interop class between C++ & CLR for BuiltinEditorResources.
	 */
	class BS_SCR_BED_EXPORT ScriptEditorBuiltin : public ScriptObject <ScriptEditorBuiltin>
	{
	public:
		SCRIPT_OBJ(EDITOR_ASSEMBLY, "BansheeEditor", "EditorBuiltin")

	private:
		ScriptEditorBuiltin(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static MonoObject* internal_getLibraryItemIcon(ProjectIcon icon, int size);
		static MonoObject* internal_getXBtnIcon();

		static MonoString* internal_GetEmptyShaderCode();
		static MonoString* internal_GetEmptyCSScriptCode();

		static MonoObject* internal_GetToolbarIcon(ToolbarIcon icon);
		static MonoObject* internal_GetLibraryWindowIcon(LibraryWindowIcon icon);
		static MonoObject* internal_GetInspectorWindowIcon(InspectorWindowIcon icon);
		static MonoObject* internal_GetSceneWindowIcon(SceneWindowIcon icon);
	};
}