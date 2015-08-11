#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsModule.h"

namespace BansheeEngine
{
	/**
	 * @brief	Keeps track of all script interop objects and handles assembly refresh.
	 */
	class BS_SCR_BE_EXPORT ScriptObjectManager : public Module <ScriptObjectManager>
	{
	public:
		ScriptObjectManager();
		~ScriptObjectManager();

		/**
		 * @brief	Registers a newly created script interop object.
		 */
		void registerScriptObject(ScriptObjectBase* instance);

		/**
		 * @brief	Unregisters a script interop object that is no longer valid.
		 */
		void unregisterScriptObject(ScriptObjectBase* instance);

		/**
		 * @brief	Reloads all script assemblies. This involves backup up managed
		 *			object data, destroying all managed objects and restoring
		 *			the objects after reload.
		 */
		void refreshAssemblies();

		/**
		 * @brief	Called once per frame. Triggers queued finalizer callbacks.
		 */
		void update();

		/**
		 * @brief	Call this when object finalizer is triggered. At the end of the frame
		 *			all objects queued with this method will have their _onManagedInstanceDeleted methods
		 *			triggered.
		 *
		 * @note	Thread safe.
		 */
		void notifyObjectFinalized(ScriptObjectBase* instance);

		/**
		 * @brief	Triggers _onManagedInstanceDeleted deleted callbacks on all objects that were finalized this frame.
		 *			This allows the native portions of those objects to properly clean up any resources.
		 */
		void processFinalizedObjects();

		/**
		 * @brief	Triggered right after a domain was reloaded. This signals the outside world that they should
		 *			update any kept Mono references as the old ones will no longer be valid.
		 */
		Event<void()> onRefreshDomainLoaded;

		/**
		 * @brief	Triggered just before the assembly refresh starts. At this point all managed
		 *			objects are still valid, but are about to be destroyed.
		 */
		Event<void()> onRefreshStarted;

		/**
		 * @brief	Triggered after the assembly refresh ends. New assemblies should be loaded at
		 *			this point.
		 */
		Event<void()> onRefreshComplete;
	private:
		Set<ScriptObjectBase*> mScriptObjects;

		Vector<ScriptObjectBase*> mFinalizedObjects[2];
		UINT32 mFinalizedQueueIdx;
		BS_MUTEX(mMutex);
	};
}