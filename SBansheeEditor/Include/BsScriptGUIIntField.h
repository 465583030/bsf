#pragma once

#include "BsScriptEditorPrerequisites.h"
#include "BsScriptGUIElement.h"

namespace BansheeEngine
{
	/**
	 * @brief	Interop class between C++ & CLR for GUIIntField.
	 */
	class BS_SCR_BED_EXPORT ScriptGUIIntField : public TScriptGUIElement<ScriptGUIIntField>
	{
	public:
		SCRIPT_OBJ(EDITOR_ASSEMBLY, "BansheeEditor", "GUIIntField")

	private:
		/**
		 * @brief	Triggered when the value in the native int field changes.
		 *
		 * @param	instance	Managed GUIIntField instance.
		 * @param	newValue	New field value.
		 */
		static void onChanged(MonoObject* instance, INT32 newValue);

		ScriptGUIIntField(MonoObject* instance, GUIIntField* intField);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_createInstance(MonoObject* instance, MonoObject* title, UINT32 titleWidth,
			MonoString* style, MonoArray* guiOptions, bool withTitle);

		static void internal_getValue(ScriptGUIIntField* nativeInstance, INT32* output);
		static void internal_setValue(ScriptGUIIntField* nativeInstance, INT32 value);
		static void internal_hasInputFocus(ScriptGUIIntField* nativeInstance, bool* output);
		static void internal_setRange(ScriptGUIIntField* nativeInstance, INT32 min, INT32 max);
		static void internal_setTint(ScriptGUIIntField* nativeInstance, Color color);

		typedef void (__stdcall *OnChangedThunkDef) (MonoObject*, INT32, MonoException**);

		static OnChangedThunkDef onChangedThunk;
	};
}