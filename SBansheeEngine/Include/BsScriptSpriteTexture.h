#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptResource.h"
#include "BsScriptObject.h"

namespace BansheeEngine
{
	/**
	 * @brief	Interop class between C++ & CLR for SpriteTexture.
	 */
	class BS_SCR_BE_EXPORT ScriptSpriteTexture : public TScriptResource<ScriptSpriteTexture, SpriteTexture>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "SpriteTexture")

		/**
		 * @brief	Returns the native internal sprite texture resource.
		 */
		static MonoObject* toManaged(const HSpriteTexture& texture);

	private:
		friend class ScriptResourceManager;

		ScriptSpriteTexture(MonoObject* instance, const HSpriteTexture& texture);

		/**
		 * @brief	Creates an empty, uninitialized managed instance of the resource interop object.
		 */
		static MonoObject* createInstance();

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_createInstance(MonoObject* instance, MonoObject* texture, Vector2* offset, Vector2* scale);
		static MonoObject* internal_GetTexture(ScriptSpriteTexture* thisPtr);
		static void internal_SetTexture(ScriptSpriteTexture* thisPtr, ScriptTexture2D* value);
		static void internal_GetOffset(ScriptSpriteTexture* thisPtr, Vector2* value);
		static void internal_SetOffset(ScriptSpriteTexture* thisPtr, Vector2* value);
		static void internal_GetScale(ScriptSpriteTexture* thisPtr, Vector2* value);
		static void internal_SetScale(ScriptSpriteTexture* thisPtr, Vector2* value);
		static UINT32 internal_GetWidth(ScriptSpriteTexture* thisPtr);
		static UINT32 internal_GetHeight(ScriptSpriteTexture* thisPtr);
	};
}