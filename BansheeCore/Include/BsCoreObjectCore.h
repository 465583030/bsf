#pragma once

#include "BsCorePrerequisites.h"
#include "BsAsyncOp.h"

namespace BansheeEngine
{
	/**
	 * @brief	Represents part of a CoreObject that is meant to be used specifically
	 *			on the core thread. 
	 *
	 * @note	Core thread only.
	 *
	 *			Different CoreObject implementations should implement this class for their
	 *			own needs.
	 */
	class BS_CORE_EXPORT CoreObjectCore
	{
	public:
		CoreObjectCore();
		virtual ~CoreObjectCore();

		/**
		 * @brief	Called on the core thread when the object is first created.
		 */
		virtual void initialize() { }

		/**
		 * @brief	Called on the core thread just before the object is destroyed.
		 */
		virtual void destroy();

	protected:
		friend class CoreObjectManager;
		friend class CoreObject;

		/**
		 * @brief	Copy internal dirty data to a memory buffer that will be used
		 *			for updating sim thread version of that data.
		 */
		virtual CoreSyncData syncFromCore(FrameAlloc* allocator) { return CoreSyncData(); }

		/**
		 * @brief	Update internal data from provided memory buffer that
		 *			was populated with data from the sim thread.
		 */
		virtual void syncToCore(const CoreSyncData& data) { }

		/**
		 * @brief	Marks the core data as dirty. This causes the syncFromCore()
		 *			method to trigger the next time objects are synced between core and sim threads.
		 *
		 * @param	flags	Optional flags in case you want to signal that only part of the
		 *					internal data is dirty. syncFromCore() will be called regardless
		 *					and it's up to the implementation to read the flags value if needed.
		 */
		void markCoreDirty(UINT32 flags = 0xFFFFFFFF) { mCoreDirtyFlags = flags; }

		/**
		 * @brief	Marks the core data as clean. Normally called right after syncFromCore()
		 *			has been called.
		 */
		void markCoreClean() { mCoreDirtyFlags = 0; }

		/**
		 * @brief	Checks is the core dirty flag set. This is used by external systems 
		 *			to know when internal data has changed and sim thread potentially needs to be notified.
		 */
		bool isCoreDirty() const { return mCoreDirtyFlags != 0; }

		UINT32 mCoreDirtyFlags;
		bool mIsDestroyed;
	};
}