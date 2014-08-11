#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsModule.h"
#include <mono/jit/jit.h>

namespace BansheeEngine
{
	class BS_SCR_BE_EXPORT ScriptResourceManager : public Module<ScriptResourceManager>
	{
	public:
		ScriptResourceManager();

		/**
		 * @note Throws an exception if resource for the handle already exists.
		 * 		 Creates a new managed instance of the object.
		 */
		ScriptTexture2D* createScriptTexture(const HTexture& resourceHandle);

		/**
		 * @note Throws an exception if resource for the handle already exists.
		 * 		 Initializes the ScriptResource with an existing managed instance.
		 */
		ScriptTexture2D* createScriptTexture(MonoObject* existingInstance, const HTexture& resourceHandle);

		/**
		 * @note Throws an exception if resource for the handle already exists.
		 * 		 Creates a new managed instance of the object.
		 */
		ScriptSpriteTexture* createScriptSpriteTexture(const HSpriteTexture& resourceHandle);

		/**
		 * @note Throws an exception if resource for the handle already exists.
		 * 		 Initializes the ScriptResource with an existing managed instance.
		 */
		ScriptSpriteTexture* createScriptSpriteTexture(MonoObject* existingInstance, const HSpriteTexture& resourceHandle);

		/**
		* @note  Throws an exception if resource for the handle already exists.
		* 		 Initializes the ScriptResource with an existing managed instance.
		*/
		ScriptManagedResource* createManagedResource(MonoObject* existingInstance, const HManagedResource& resourceHandle);

		/**
		 * @note Returns nullptr if script resource doesn't exist.
		 */
		ScriptTexture2D* getScriptTexture(const HTexture& resourceHandle);

		/**
		 * @note Returns nullptr if script resource doesn't exist.
		 */
		ScriptSpriteTexture* getScriptSpriteTexture(const HSpriteTexture& resourceHandle);

		/**
		 * @note Returns nullptr if script resource doesn't exist.
		 */
		ScriptManagedResource* getScriptManagedResource(const HManagedResource& resourceHandle);

		/**
		 * @note Returns nullptr if script resource doesn't exist.
		 */
		ScriptResourceBase* getScriptResource(const String& UUID);

		void destroyScriptResource(ScriptResourceBase* resource);

	private:
		UnorderedMap<String, ScriptResourceBase*> mScriptResources;
		MonoClass* mTextureClass;
		MonoClass* mSpriteTextureClass;

		void throwExceptionIfInvalidOrDuplicate(const String& uuid) const;
	};
}