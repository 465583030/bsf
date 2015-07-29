#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "BsGUIElementStyle.h"
#include "BsMonoClass.h"
#include "BsScriptGUIElementStateStyle.h"
#include "BsScriptFont.h"

namespace BansheeEngine
{
	/**
	 * @brief	Interop class between C++ & CLR for GUIElementStyle.
	 */
	class BS_SCR_BE_EXPORT ScriptGUIElementStyle : public ScriptObject<ScriptGUIElementStyle>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "GUIElementStyle")

		~ScriptGUIElementStyle();

		/**
		 * @brief	Returns the wrapped GUIElementStyle instance.
		 */
		GUIElementStyle* getInternalValue() const { return mElementStyle; }

	private:
		/**
		 * @brief	Creates the interop object with a default style.
		 */
		ScriptGUIElementStyle(MonoObject* instance, const String& name);

		/**
		 * @brief	Creates the interop object referencing an existing style.
		 */
		ScriptGUIElementStyle(MonoObject* instance, const String& name, GUIElementStyle* externalStyle);

		String mName;
		GUIElementStyle* mElementStyle;
		bool mOwnsStyle;

		ScriptFont* mFont;
		ScriptGUIElementStateStyle* mNormal;
		ScriptGUIElementStateStyle* mHover;
		ScriptGUIElementStateStyle* mActive;
		ScriptGUIElementStateStyle* mFocused;
		ScriptGUIElementStateStyle* mNormalOn;
		ScriptGUIElementStateStyle* mHoverOn;
		ScriptGUIElementStateStyle* mActiveOn;
		ScriptGUIElementStateStyle* mFocusedOn;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_createInstance(MonoObject* instance, MonoString* name);
		static void internal_createInstanceExternal(MonoObject* instance, MonoString* name, GUIElementStyle* externalStyle);
		static void internal_addSubStyle(ScriptGUIElementStyle* nativeInstance, MonoString* guiType, MonoString* styleName);

		static void internal_GetFont(ScriptGUIElementStyle* nativeInstance, MonoObject** value);
		static void internal_SetFont(ScriptGUIElementStyle* nativeInstance, MonoObject* value);

		static void internal_GetFontSize(ScriptGUIElementStyle* nativeInstance, UINT32* value);
		static void internal_SetFontSize(ScriptGUIElementStyle* nativeInstance, UINT32 value);
		static void internal_GetTextHorzAlign(ScriptGUIElementStyle* nativeInstance, TextHorzAlign*value);
		static void internal_SetTextHorzAlign(ScriptGUIElementStyle* nativeInstance, TextHorzAlign value);
		static void internal_GetTextVertAlign(ScriptGUIElementStyle* nativeInstance, TextVertAlign* value);
		static void internal_SetTextVertAlign(ScriptGUIElementStyle* nativeInstance, TextVertAlign value);
		static void internal_GetImagePosition(ScriptGUIElementStyle* nativeInstance, GUIImagePosition* value);
		static void internal_SetImagePosition(ScriptGUIElementStyle* nativeInstance, GUIImagePosition value);
		static void internal_GetWordWrap(ScriptGUIElementStyle* nativeInstance, bool* value);
		static void internal_SetWordWrap(ScriptGUIElementStyle* nativeInstance, bool value);

		static void internal_GetNormal(ScriptGUIElementStyle* nativeInstance, MonoObject** value);
		static void internal_SetNormal(ScriptGUIElementStyle* nativeInstance, MonoObject* value);
		static void internal_GetHover(ScriptGUIElementStyle* nativeInstance, MonoObject** value);
		static void internal_SetHover(ScriptGUIElementStyle* nativeInstance, MonoObject* value);
		static void internal_GetActive(ScriptGUIElementStyle* nativeInstance, MonoObject** value);
		static void internal_SetActive(ScriptGUIElementStyle* nativeInstance, MonoObject* value);
		static void internal_GetFocused(ScriptGUIElementStyle* nativeInstance, MonoObject** value);
		static void internal_SetFocused(ScriptGUIElementStyle* nativeInstance, MonoObject* value);

		static void internal_GetNormalOn(ScriptGUIElementStyle* nativeInstance, MonoObject** value);
		static void internal_SetNormalOn(ScriptGUIElementStyle* nativeInstance, MonoObject* value);
		static void internal_GetHoverOn(ScriptGUIElementStyle* nativeInstance, MonoObject** value);
		static void internal_SetHoverOn(ScriptGUIElementStyle* nativeInstance, MonoObject* value);
		static void internal_GetActiveOn(ScriptGUIElementStyle* nativeInstance, MonoObject** value);
		static void internal_SetActiveOn(ScriptGUIElementStyle* nativeInstance, MonoObject* value);
		static void internal_GetFocusedOn(ScriptGUIElementStyle* nativeInstance, MonoObject** value);
		static void internal_SetFocusedOn(ScriptGUIElementStyle* nativeInstance, MonoObject* value);

		static void internal_GetBorder(ScriptGUIElementStyle* nativeInstance, RectOffset* value);
		static void internal_SetBorder(ScriptGUIElementStyle* nativeInstance, RectOffset* value);
		static void internal_GetMargins(ScriptGUIElementStyle* nativeInstance, RectOffset* value);
		static void internal_SetMargins(ScriptGUIElementStyle* nativeInstance, RectOffset* value);
		static void internal_GetContentOffset(ScriptGUIElementStyle* nativeInstance, RectOffset* value);
		static void internal_SetContentOffset(ScriptGUIElementStyle* nativeInstance, RectOffset* value);

		static void internal_GetWidth(ScriptGUIElementStyle* nativeInstance, UINT32* value);
		static void internal_SetWidth(ScriptGUIElementStyle* nativeInstance, UINT32 value);
		static void internal_GetHeight(ScriptGUIElementStyle* nativeInstance, UINT32* value);
		static void internal_SetHeight(ScriptGUIElementStyle* nativeInstance, UINT32 value);
		static void internal_GetMinWidth(ScriptGUIElementStyle* nativeInstance, UINT32* value);
		static void internal_SetMinWidth(ScriptGUIElementStyle* nativeInstance, UINT32 value);
		static void internal_GetMaxWidth(ScriptGUIElementStyle* nativeInstance, UINT32* value);
		static void internal_SetMaxWidth(ScriptGUIElementStyle* nativeInstance, UINT32 value);
		static void internal_GetMinHeight(ScriptGUIElementStyle* nativeInstance, UINT32* value);
		static void internal_SetMinHeight(ScriptGUIElementStyle* nativeInstance, UINT32 value);
		static void internal_GetMaxHeight(ScriptGUIElementStyle* nativeInstance, UINT32* value);
		static void internal_SetMaxHeight(ScriptGUIElementStyle* nativeInstance, UINT32 value);
		static void internal_GetFixedWidth(ScriptGUIElementStyle* nativeInstance, bool* value);
		static void internal_SetFixedWidth(ScriptGUIElementStyle* nativeInstance, bool value);
		static void internal_GetFixedHeight(ScriptGUIElementStyle* nativeInstance, bool* value);
		static void internal_SetFixedHeight(ScriptGUIElementStyle* nativeInstance, bool value);
	};
}