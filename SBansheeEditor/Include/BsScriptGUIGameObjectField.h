#pragma once

#include "BsScriptEditorPrerequisites.h"
#include "BsScriptGUIElement.h"

namespace BansheeEngine
{
	/**
	 * @brief	Interop class between C++ & CLR for GUIGameObjectField.
	 */
	class BS_SCR_BED_EXPORT ScriptGUIGameObjectField : public TScriptGUIElement<ScriptGUIGameObjectField>
	{
	public:
		SCRIPT_OBJ(EDITOR_ASSEMBLY, "BansheeEditor", "GUIGameObjectField")

	private:
		/**
		 * @brief	Triggered when the value in the native game object field changes.
		 *
		 * @param	instance	Managed GUIGameObjectField instance.
		 * @param	newValue	New field value.
		 */
		static void onChanged(MonoObject* instance, const HGameObject& newValue);

		/**
		 * @brief	Retrieves a managed instance of the specified native game object.
		 *			Will return null if one doesn't exist.
		 */
		static MonoObject* nativeToManagedGO(const HGameObject& instance);

		ScriptGUIGameObjectField(MonoObject* instance, GUIGameObjectField* GOField);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_createInstance(MonoObject* instance, MonoReflectionType* type, MonoObject* title, UINT32 titleWidth,
			MonoString* style, MonoArray* guiOptions, bool withTitle);

		static void internal_getValue(ScriptGUIGameObjectField* nativeInstance, MonoObject** output);
		static void internal_setValue(ScriptGUIGameObjectField* nativeInstance, MonoObject* value);
		static void internal_setTint(ScriptGUIGameObjectField* nativeInstance, Color color);


		typedef void(__stdcall *OnChangedThunkDef) (MonoObject*, MonoObject*, MonoException**);

		static OnChangedThunkDef onChangedThunk;
	};
}