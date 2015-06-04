#include "BsEditorScriptManager.h"
#include "BsScriptEditorWindow.h"
#include "BsScriptObjectManager.h"
#include "BsMonoAssembly.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"
#include "BsScriptAssemblyManager.h"
#include "BsScriptGizmoManager.h"
#include "BsScriptHandleManager.h"
#include "BsScriptHandleSliderManager.h"
#include "BsScriptDragDropManager.h"
#include "BsScriptProjectLibrary.h"
#include "BsMenuItemManager.h"
#include "BsScriptFolderMonitor.h"
#include "BsTime.h"
#include "BsMath.h"
#include "BsEditorApplication.h"

namespace BansheeEngine
{
	const float EditorScriptManager::EDITOR_UPDATE_RATE = 1.0f/60.0f; // Seconds
	bool EditorScriptManager::mDebugRefresh = false;

	EditorScriptManager::EditorScriptManager()
		:mEditorAssembly(nullptr), mProgramEdClass(nullptr), mUpdateMethod(nullptr)
	{
		loadMonoTypes();
		ScriptAssemblyManager::instance().loadAssemblyInfo(EDITOR_ASSEMBLY);

		ScriptHandleSliderManager::startUp();
		ScriptGizmoManager::startUp(ScriptAssemblyManager::instance());
		HandleManager::startUp<ScriptHandleManager>(ScriptAssemblyManager::instance());
		ScriptDragDropManager::startUp();
		ScriptProjectLibrary::startUp();
		MenuItemManager::startUp(ScriptAssemblyManager::instance());
		ScriptFolderMonitorManager::startUp();

		mOnDomainLoadConn = ScriptObjectManager::instance().onRefreshDomainLoaded.connect(std::bind(&EditorScriptManager::loadMonoTypes, this));
		mOnAssemblyRefreshDoneConn = ScriptObjectManager::instance().onRefreshComplete.connect(std::bind(&EditorScriptManager::onAssemblyRefreshDone, this));
		triggerOnInitialize();
		
		// Initial update
		mLastUpdateTime = gTime().getTime();
		mUpdateMethod->invoke(nullptr, nullptr);
	}

	EditorScriptManager::~EditorScriptManager()
	{
		mOnDomainLoadConn.disconnect();
		mOnAssemblyRefreshDoneConn.disconnect();

		ScriptFolderMonitorManager::shutDown();
		MenuItemManager::shutDown();
		ScriptProjectLibrary::shutDown();
		ScriptDragDropManager::shutDown();
		ScriptHandleSliderManager::shutDown();
		HandleManager::shutDown();
		ScriptGizmoManager::shutDown();
	}

	void EditorScriptManager::update()
	{
		float curTime = gTime().getTime();
		float diff = curTime - mLastUpdateTime;

		if (diff > EDITOR_UPDATE_RATE)
		{
			mUpdateMethod->invoke(nullptr, nullptr);

			INT32 numUpdates = Math::floorToInt(diff / EDITOR_UPDATE_RATE);
			mLastUpdateTime += numUpdates * EDITOR_UPDATE_RATE;
		}

		if (mDebugRefresh)
		{
			ScriptObjectManager::instance().refreshAssemblies();
			mDebugRefresh = false;
		}

		ScriptGizmoManager::instance().update();
		ScriptDragDropManager::instance().update();
		ScriptFolderMonitorManager::instance().update();
	}

	void EditorScriptManager::debug_refreshAssembly()
	{
		mDebugRefresh = true;
	}

	void EditorScriptManager::triggerOnInitialize()
	{
		const String ASSEMBLY_ENTRY_POINT = "Program::OnInitialize";
		mEditorAssembly->invoke(ASSEMBLY_ENTRY_POINT);
	}

	void EditorScriptManager::onAssemblyRefreshDone()
	{
		triggerOnInitialize();
	}

	void EditorScriptManager::loadMonoTypes()
	{
		const String editorAssemblyPath = gEditorApplication().getEditorAssemblyPath().toString();
		mEditorAssembly = &MonoManager::instance().loadAssembly(editorAssemblyPath, EDITOR_ASSEMBLY);

		// TODO - Load Editor script assembly (gEditorApplication.getEditorScriptAssemblyPath())

		mProgramEdClass = mEditorAssembly->getClass("BansheeEditor", "Program");
		mUpdateMethod = mProgramEdClass->getMethod("OnEditorUpdate");

		ScriptEditorWindow::clearRegisteredEditorWindow();
		ScriptEditorWindow::registerManagedEditorWindows();

		// DEBUG ONLY
		MonoClass* debugWindowClass = mEditorAssembly->getClass("BansheeEditor", "DebugWindow");
		debugWindowClass->addInternalCall("Internal_RefreshAssembly", &EditorScriptManager::debug_refreshAssembly);
	}
}