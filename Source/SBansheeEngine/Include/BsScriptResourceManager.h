//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsModule.h"
#include <mono/jit/jit.h>

namespace BansheeEngine
{
	/** @addtogroup SBansheeEngine
	 *  @{
	 */

	/**
	 * Handles creation and lookup of script interop objects for resources. Since resources can be created in native code
	 * yet used by managed code this manager provides lookups to find managed equivalents of native resources.
	 */
	class BS_SCR_BE_EXPORT ScriptResourceManager : public Module<ScriptResourceManager>
	{
	public:
		ScriptResourceManager();
		~ScriptResourceManager();

		/**
		 * Creates a new managed instance and interop object for the specified resource.
		 *
		 * @param[in]	resourceHandle	Native resource to wrap in a managed instance.
		 * @param[out]	out				Output interop object corresponding to the new managed instance.
		 *
		 * @note	Throws an exception if a managed instance for the provided resource already exists.
		 */
		template<class RetType, class InType>
		void createScriptResource(const ResourceHandle<InType>& resourceHandle, RetType** out);

		/**
		 * Creates a new interop object for the specified resource using an existing managed instance.
		 *
		 * @param[in]	existingInstance	Existing managed instance. Caller must ensure the managed instance matches the
		 *									native resource type.
		 * @param[in]	resourceHandle		Native resource to link to the managed instance.
		 * @param[out]	out					Output interop object corresponding to the new managed instance.
		 *
		 * @note	Throws an exception if a managed instance for the provided resource already exists.
		 */
		template<class RetType, class InType>
		void createScriptResource(MonoObject* existingInstance, const ResourceHandle<InType>& resourceHandle, RetType** out);

		/**
		 * Attempts to find an existing interop object for the specified resource, and optionally creates a new one if one
		 * cannot be found.
		 * 
		 * @param[in]	resourceHandle	Resource to search for.
		 * @param[out]	out				Found or created interop object containing the resource.
		 * @param[in]	create			If a resource cannot be found new one will be created when this is true. If false
		 *								and the resource doesn't exist it will be null.
		 */
		template<class RetType, class InType>
		void getScriptResource(const ResourceHandle<InType>& resourceHandle, RetType** out, bool create = false);

		/**
		 * Creates a new managed instance and interop object for the specified string table.
		 *
		 * @param[in]	resourceHandle	Native string table resource to wrap in a managed instance.
		 * @param[out]	out				Output string table interop object corresponding to the new managed instance.
		 *
		 * @note	Throws an exception if a managed instance for the provided resource already exists.
		 */
		template<>
		void createScriptResource(const ResourceHandle<StringTable>& resourceHandle, ScriptStringTable** out);

		/**
		 * Creates a new managed instance and interop object for the specified resource.
		 *
		 * @param[in]	resourceHandle	Native resource to wrap in a managed instance.
		 * @param[out]	out				Output interop object corresponding to the new managed instance.
		 *
		 * @note	Throws an exception if a managed instance for the provided resource already exists.
		 */
		template<>
		void createScriptResource(const HResource& resourceHandle, ScriptResourceBase** out);

		/**
		 * Attempts to find a resource interop object for a resource with the specified UUID. Returns null if the object
		 * cannot be found.
		 */
		ScriptResourceBase* getScriptResource(const String& UUID);

		/**
		 * Deletes the provided resource interop objects. All resource interop objects should be deleted using this method.
		 */
		void destroyScriptResource(ScriptResourceBase* resource);

	private:
		/**	Triggered when the native resource has been unloaded and therefore destroyed. */
		void onResourceDestroyed(const String& UUID);

		/**	Throws an exception if the provided UUID already exists in the interop object lookup table. */
		void throwExceptionIfInvalidOrDuplicate(const String& uuid) const;

		UnorderedMap<String, ScriptResourceBase*> mScriptResources;
		HEvent mResourceDestroyedConn;
	};

	/** @} */
}