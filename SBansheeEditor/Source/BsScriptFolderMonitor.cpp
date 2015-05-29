#include "BsScriptFolderMonitor.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoMethod.h"
#include "BsMonoUtil.h"

using namespace std::placeholders;

namespace BansheeEngine
{
	ScriptFolderMonitor::OnModifiedThunkDef ScriptFolderMonitor::OnModifiedThunk;
	ScriptFolderMonitor::OnModifiedThunkDef ScriptFolderMonitor::OnAddedThunk;
	ScriptFolderMonitor::OnModifiedThunkDef ScriptFolderMonitor::OnRemovedThunk;
	ScriptFolderMonitor::OnRenamedThunkDef ScriptFolderMonitor::OnRenamedThunk;

	ScriptFolderMonitor::ScriptFolderMonitor(MonoObject* instance, FolderMonitor* monitor)
		:ScriptObject(instance), mMonitor(monitor)
	{
		if (mMonitor != nullptr)
		{
			ScriptFolderMonitorManager::instance()._registerMonitor(this);

			mMonitor->onAdded.connect(std::bind(&ScriptFolderMonitor::onMonitorFileAdded, this, _1));
			mMonitor->onRemoved.connect(std::bind(&ScriptFolderMonitor::onMonitorFileRemoved, this, _1));
			mMonitor->onModified.connect(std::bind(&ScriptFolderMonitor::onMonitorFileModified, this, _1));
			mMonitor->onRenamed.connect(std::bind(&ScriptFolderMonitor::onMonitorFileRenamed, this, _1, _2));
		}
	}

	ScriptFolderMonitor::~ScriptFolderMonitor()
	{
		destroy();
	}

	void ScriptFolderMonitor::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptFolderMonitor::internal_CreateInstance);
		metaData.scriptClass->addInternalCall("Internal_Destroy", &ScriptFolderMonitor::internal_Destroy);

		OnModifiedThunk = (OnModifiedThunkDef)metaData.scriptClass->getMethod("Internal_DoOnModified", 1)->getThunk();
		OnAddedThunk = (OnModifiedThunkDef)metaData.scriptClass->getMethod("Internal_DoOnAdded", 1)->getThunk();
		OnRemovedThunk = (OnModifiedThunkDef)metaData.scriptClass->getMethod("Internal_DoOnRemoved", 1)->getThunk();
		OnRenamedThunk = (OnRenamedThunkDef)metaData.scriptClass->getMethod("Internal_DoOnRenamed", 2)->getThunk();
	}

	void ScriptFolderMonitor::internal_CreateInstance(MonoObject* instance, MonoString* folder)
	{
		FolderMonitor* monitor = nullptr;
		if (folder != nullptr)
		{
			Path folderPath = MonoUtil::monoToWString(folder);

			FolderChange folderChanges = (FolderChange)((UINT32)FolderChange::FileName | (UINT32)FolderChange::DirName |
				(UINT32)FolderChange::Creation | (UINT32)FolderChange::LastWrite);

			monitor = bs_new<FolderMonitor>();
			monitor->startMonitor(folderPath, true, folderChanges);
		}

		new (bs_alloc<ScriptFolderMonitor>()) ScriptFolderMonitor(instance, monitor);
	}

	void ScriptFolderMonitor::internal_Destroy(ScriptFolderMonitor* thisPtr)
	{
		thisPtr->destroy();
	}

	void ScriptFolderMonitor::onMonitorFileModified(const Path& path)
	{
		MonoString* monoPath = MonoUtil::wstringToMono(MonoManager::instance().getDomain(), path.toWString());

		MonoException* exception = nullptr;
		OnModifiedThunk(getManagedInstance(), monoPath, &exception);

		MonoUtil::throwIfException(exception);
	}

	void ScriptFolderMonitor::onMonitorFileAdded(const Path& path)
	{
		MonoString* monoPath = MonoUtil::wstringToMono(MonoManager::instance().getDomain(), path.toWString());

		MonoException* exception = nullptr;
		OnAddedThunk(getManagedInstance(), monoPath, &exception);

		MonoUtil::throwIfException(exception);
	}

	void ScriptFolderMonitor::onMonitorFileRemoved(const Path& path)
	{
		MonoString* monoPath = MonoUtil::wstringToMono(MonoManager::instance().getDomain(), path.toWString());

		MonoException* exception = nullptr;
		OnRemovedThunk(getManagedInstance(), monoPath, &exception);

		MonoUtil::throwIfException(exception);
	}

	void ScriptFolderMonitor::onMonitorFileRenamed(const Path& from, const Path& to)
	{
		MonoString* monoPathFrom = MonoUtil::wstringToMono(MonoManager::instance().getDomain(), from.toWString());
		MonoString* monoPathTo = MonoUtil::wstringToMono(MonoManager::instance().getDomain(), to.toWString());

		MonoException* exception = nullptr;
		OnRenamedThunk(getManagedInstance(), monoPathFrom, monoPathTo, &exception);

		MonoUtil::throwIfException(exception);
	}

	void ScriptFolderMonitor::update()
	{
		mMonitor->_update();
	}

	void ScriptFolderMonitor::destroy()
	{
		if (mMonitor != nullptr)
		{
			mMonitor->stopMonitorAll();
			bs_delete(mMonitor);
			mMonitor = nullptr;

			ScriptFolderMonitorManager::instance()._unregisterMonitor(this);
		}
	}

	void ScriptFolderMonitorManager::update()
	{
		for (auto& monitor : mMonitors)
			monitor->update();
	}

	void ScriptFolderMonitorManager::_registerMonitor(ScriptFolderMonitor* monitor)
	{
		mMonitors.insert(monitor);
	}

	void ScriptFolderMonitorManager::_unregisterMonitor(ScriptFolderMonitor* monitor)
	{
		mMonitors.erase(monitor);
	}
}