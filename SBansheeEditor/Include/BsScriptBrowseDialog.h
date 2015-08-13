#pragma once

#include "BsScriptEditorPrerequisites.h"
#include "BsScriptObject.h"

namespace BansheeEngine
{
	/**
	 * @brief	Interop class between C++ & CLR for browse dialog methods in Platform.
	 */
	class BS_SCR_BED_EXPORT ScriptBrowseDialog : public ScriptObject<ScriptBrowseDialog>
	{
	public:
		SCRIPT_OBJ(EDITOR_ASSEMBLY, "BansheeEditor", "BrowseDialog")

	private:

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static bool internal_OpenFile(MonoString* defaultFolder, MonoString* filterList, bool allowMultiselect, MonoArray** outPaths);
		static bool internal_OpenFolder(MonoString* defaultFolder, MonoString* filterList, MonoString** outPath);
		static bool internal_SaveFile(MonoString* defaultFolder, MonoString* filterList, MonoString** outPath);
	};
}