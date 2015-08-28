#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsModule.h"

namespace BansheeEngine
{
	/**
	 * @brief	Keeps track of all loaded managed resources.
	 */
	class BS_SCR_BE_EXPORT ManagedResourceManager : public Module <ManagedResourceManager>
	{
	public:
		ManagedResourceManager();
		~ManagedResourceManager();

		/**   
		 * @brief	 Unloads all managed resources.
		 */
		void clear();

		/**
		 * @brief	Register a newly created managed resource.
		 */
		void registerManagedResource(const HManagedResource& resource);

		/**
		 * @brief	Unregister a managed resource that's about to be destroyed.
		 */
		void unregisterManagedResource(const HManagedResource& resource);

	private:
		UnorderedMap<String, HManagedResource> mResources;
	};
}