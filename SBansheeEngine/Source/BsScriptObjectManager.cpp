#include "BsScriptObjectManager.h"
#include "BsScriptObject.h"
#include "BsMonoManager.h"
#include "BsScriptAssemblyManager.h"

namespace BansheeEngine
{
	ScriptObjectManager::ScriptObjectManager()
		:mFinalizedQueueIdx(0)
	{

	}

	ScriptObjectManager::~ScriptObjectManager()
	{
		processFinalizedObjects();
	}

	void ScriptObjectManager::registerScriptObject(ScriptObjectBase* instance)
	{
		mScriptObjects.insert(instance);
	}

	void ScriptObjectManager::unregisterScriptObject(ScriptObjectBase* instance)
	{
		mScriptObjects.erase(instance);
	}

	void ScriptObjectManager::refreshAssemblies()
	{
		Map<ScriptObjectBase*, ScriptObjectBackup> backupData;

		onRefreshStarted();

		for (auto& scriptObject : mScriptObjects)
			backupData[scriptObject] = scriptObject->beginRefresh();

		MonoManager::instance().unloadScriptDomain();
		// Unload script domain should trigger finalizers on everything, but since we usually delay
		// their processing we need to manually trigger it here.
		processFinalizedObjects();

		for (auto& scriptObject : mScriptObjects)
			assert(scriptObject->isPersistent() && "Non-persistent ScriptObject alive after domain unload.");

		for (auto& scriptObject : mScriptObjects)
			scriptObject->_clearManagedInstance();

		MonoManager::instance().loadScriptDomain();
		ScriptAssemblyManager::instance().refreshAssemblyInfo();

		onRefreshDomainLoaded();

		for (auto& scriptObject : mScriptObjects)
			scriptObject->_restoreManagedInstance();

		for (auto& scriptObject : mScriptObjects)
			scriptObject->endRefresh(backupData[scriptObject]);

		onRefreshComplete();
	}

	void ScriptObjectManager::notifyObjectFinalized(ScriptObjectBase* instance)
	{
		UINT32 idx = mFinalizedQueueIdx.load(std::memory_order_relaxed);
		mFinalizedObjects[idx].push_back(instance);
	}

	void ScriptObjectManager::update()
	{
		processFinalizedObjects();
	}

	void ScriptObjectManager::processFinalizedObjects()
	{
		UINT32 readQueueIdx = mFinalizedQueueIdx.fetch_xor(0x1, std::memory_order_relaxed);

		for (auto& finalizedObj : mFinalizedObjects[readQueueIdx])
			finalizedObj->_onManagedInstanceDeleted();

		mFinalizedObjects[readQueueIdx].clear();
	}
}