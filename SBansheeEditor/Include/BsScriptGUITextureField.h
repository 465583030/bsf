#pragma once

#include "BsScriptEditorPrerequisites.h"
#include "BsScriptGUIElement.h"

namespace BansheeEngine
{
	/**
	 * @brief	Interop class between C++ & CLR for GUITextureField.
	 */
	class BS_SCR_BED_EXPORT ScriptGUITextureField : public TScriptGUIElement <ScriptGUITextureField>
	{
	public:
		SCRIPT_OBJ(EDITOR_ASSEMBLY, "BansheeEditor", "GUITextureField")

	private:
		/**
		 * @brief	Triggered when the value in the native texture field changes.
		 *
		 * @param	instance	Managed GUITextureField instance.
		 * @param	newHandle	Handle of the new texture.
		 */
		static void onChanged(MonoObject* instance, const WeakResourceHandle<Texture>& newHandle);

		/**
		 * @brief	Retrieves a managed instance of the specified native texture.
		 *			Will return null if one doesn't exist.
		 */
		static MonoObject* nativeToManagedResource(const HTexture& instance);

		ScriptGUITextureField(MonoObject* instance, GUITextureField* textureField);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/

		static void internal_createInstance(MonoObject* instance, MonoObject* title, UINT32 titleWidth,
			MonoString* style, MonoArray* guiOptions, bool withTitle);

		static void internal_getValue(ScriptGUITextureField* nativeInstance, MonoObject** output);
		static void internal_setValue(ScriptGUITextureField* nativeInstance, MonoObject* value);
		static void internal_getValueRef(ScriptGUITextureField* nativeInstance, MonoObject** output);
		static void internal_setValueRef(ScriptGUITextureField* nativeInstance, MonoObject* value);
		static void internal_setTint(ScriptGUITextureField* nativeInstance, Color color);

		typedef void(__stdcall *OnChangedThunkDef) (MonoObject*, MonoObject*, MonoException**);

		static OnChangedThunkDef onChangedThunk;
	};
}