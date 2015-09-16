#pragma once

#include "BsScriptEditorPrerequisites.h"
#include "BsScriptGUIElement.h"

namespace BansheeEngine
{
	/**
	 * @brief	Interop class between C++ & CLR for GUIListBoxField.
	 */
	class BS_SCR_BED_EXPORT ScriptGUIListBoxField : public TScriptGUIElement<ScriptGUIListBoxField>
	{
	public:
		SCRIPT_OBJ(EDITOR_ASSEMBLY, "BansheeEditor", "GUIListBoxField")

	private:
		/**
		 * @brief	Triggered when the value in the native list box selection changes.
		 *
		 * @param	instance	Managed GUIListBoxField instance.
		 * @param	newIndex	New selection index.
		 */
		static void onSelectionChanged(MonoObject* instance, UINT32 newIndex);

		ScriptGUIListBoxField(MonoObject* instance, GUIListBoxField* listBoxField);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_createInstance(MonoObject* instance, MonoArray* elements, MonoObject* title, UINT32 titleWidth,
			MonoString* style, MonoArray* guiOptions, bool withTitle);

		static void internal_setElements(ScriptGUIListBoxField* nativeInstance, MonoArray* elements);
		static UINT32 internal_getValue(ScriptGUIListBoxField* nativeInstance);
		static void internal_setValue(ScriptGUIListBoxField* nativeInstance, UINT32 value);
		static void internal_setTint(ScriptGUIListBoxField* nativeInstance, Color color);

		typedef void(__stdcall *OnSelectionChangedThunkDef) (MonoObject*, UINT32, MonoException**);

		static OnSelectionChangedThunkDef onSelectionChangedThunk;
	};
}