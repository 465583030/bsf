#pragma once

#include "BsScriptEditorPrerequisites.h"
#include "BsScriptGUIElement.h"

namespace BansheeEngine
{
	class BS_SCR_BED_EXPORT ScriptGUITextField : public TScriptGUIElement<ScriptGUITextField>
	{
	public:
		SCRIPT_OBJ(EDITOR_ASSEMBLY, "BansheeEditor", "GUITextField")

	private:
		static void internal_createInstance(MonoObject* instance, bool multiline, MonoObject* title, UINT32 titleWidth,
			MonoString* style, MonoArray* guiOptions, bool withTitle);

		static void internal_getValue(ScriptGUITextField* nativeInstance, MonoString** output);
		static void internal_setValue(ScriptGUITextField* nativeInstance, MonoString* value);
		static void internal_hasInputFocus(ScriptGUITextField* nativeInstance, bool* output);
		static void internal_setTint(ScriptGUITextField* nativeInstance, Color color);

		static void onChanged(MonoObject* instance, const WString& newValue);

		ScriptGUITextField(MonoObject* instance, GUITextField* textField);

		typedef void(__stdcall *OnChangedThunkDef) (MonoObject*, MonoString*, MonoException**);

		static OnChangedThunkDef onChangedThunk;
	};
}