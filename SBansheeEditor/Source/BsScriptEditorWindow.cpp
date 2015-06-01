#include "BsScriptEditorWindow.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"
#include "BsMonoManager.h"
#include "BsMonoUtil.h"
#include "BsEditorWidget.h"
#include "BsEditorWidgetManager.h"
#include "BsEditorWidgetContainer.h"
#include "BsMonoAssembly.h"
#include "BsScriptObjectManager.h"
#include "BsScriptGUILayout.h"

using namespace std::placeholders;

namespace BansheeEngine
{
	UnorderedMap<String, ScriptEditorWindow::EditorWindowHandle> ScriptEditorWindow::OpenScriptEditorWindows;
	Vector<String> ScriptEditorWindow::AvailableWindowTypes;
	MonoMethod* ScriptEditorWindow::onResizedMethod = nullptr;
	MonoMethod* ScriptEditorWindow::onFocusChangedMethod = nullptr;
	MonoField* ScriptEditorWindow::guiPanelField = nullptr;

	ScriptEditorWindow::ScriptEditorWindow(ScriptEditorWidget* editorWidget)
		:ScriptObject(editorWidget->getManagedInstance()), mName(editorWidget->getName()), mEditorWidget(editorWidget), 
		mRefreshInProgress(false), mIsDestroyed(false)
	{
		mOnWidgetResizedConn = editorWidget->onResized.connect(std::bind(&ScriptEditorWindow::onWidgetResized, this, _1, _2));
		mOnFocusChangedConn = editorWidget->onFocusChanged.connect(std::bind(&ScriptEditorWindow::onFocusChanged, this, _1));

		mOnAssemblyRefreshStartedConn = ScriptObjectManager::instance().onRefreshStarted.connect(std::bind(&ScriptEditorWindow::onAssemblyRefreshStarted, this));
	}

	ScriptEditorWindow::~ScriptEditorWindow()
	{
		// TODO - This probably need to be called whenever we close a window, and not when script class is destructed
		mOnWidgetResizedConn.disconnect();
		mOnFocusChangedConn.disconnect();

		mOnAssemblyRefreshStartedConn.disconnect();
	}

	void ScriptEditorWindow::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateOrGetInstance", &ScriptEditorWindow::internal_createOrGetInstance);
		metaData.scriptClass->addInternalCall("Internal_GetWidth", &ScriptEditorWindow::internal_getWidth);
		metaData.scriptClass->addInternalCall("Internal_GetHeight", &ScriptEditorWindow::internal_getHeight);
		metaData.scriptClass->addInternalCall("Internal_HasFocus", &ScriptEditorWindow::internal_hasFocus);
		metaData.scriptClass->addInternalCall("Internal_ScreenToWindowPos", &ScriptEditorWindow::internal_screenToWindowPos);
		metaData.scriptClass->addInternalCall("Internal_WindowToScreenPos", &ScriptEditorWindow::internal_windowToScreenPos);

		onResizedMethod = metaData.scriptClass->getMethod("WindowResized", 2);
		onFocusChangedMethod = metaData.scriptClass->getMethod("FocusChanged", 1);

		guiPanelField = metaData.scriptClass->getField("GUI");
	}

	MonoObject* ScriptEditorWindow::internal_createOrGetInstance(MonoString* ns, MonoString* typeName)
	{
		String strTypeName = toString(MonoUtil::monoToWString(typeName));
		String strNamespace = toString(MonoUtil::monoToWString(ns));
		String fullName = strNamespace + "." + strTypeName;

		auto findIter = OpenScriptEditorWindows.find(fullName);
		if(findIter != OpenScriptEditorWindows.end())
			return findIter->second.nativeObj->mManagedInstance;

		EditorWidgetManager::instance().open(fullName); // This will trigger openEditorWidgetCallback and update OpenScriptEditorWindows

		auto findIter2 = OpenScriptEditorWindows.find(fullName);
		if(findIter2 != OpenScriptEditorWindows.end())
			return findIter2->second.nativeObj->mManagedInstance;

		return nullptr;
	}

	EditorWidgetBase* ScriptEditorWindow::getEditorWidget() const 
	{ 
		return mEditorWidget; 
	}

	void ScriptEditorWindow::_onManagedInstanceDeleted()
	{
		if (!mRefreshInProgress)
		{
			unregisterScriptEditorWindow(mName);

			ScriptObject::_onManagedInstanceDeleted();
		}
		else
			mManagedInstance = nullptr;
	}

	ScriptObjectBackup ScriptEditorWindow::beginRefresh()
	{
		mRefreshInProgress = true;

		auto iterFind = OpenScriptEditorWindows.find(mName);
		if (iterFind != OpenScriptEditorWindows.end())
		{
			EditorWindowHandle handle = iterFind->second;

			mono_gchandle_free(handle.gcHandle);
			iterFind->second.gcHandle = 0;
		}

		return PersistentScriptObjectBase::beginRefresh();
	}

	void ScriptEditorWindow::endRefresh(const ScriptObjectBackup& backupData)
	{
		mRefreshInProgress = false;

		if (!isDestroyed())
			mManagedInstance = mEditorWidget->getManagedInstance();
		else
			mManagedInstance = nullptr;

		if (mManagedInstance != nullptr)
		{
			auto iterFind = OpenScriptEditorWindows.find(mName);
			if (iterFind != OpenScriptEditorWindows.end())
			{
				iterFind->second.gcHandle = mono_gchandle_new(mManagedInstance, false);
			}

			mEditorWidget->triggerOnInitialize();
		}
		else
		{
			// We couldn't restore managed instance because window class cannot be found
			_onManagedInstanceDeleted();
		}

		PersistentScriptObjectBase::endRefresh(backupData);
	}

	void ScriptEditorWindow::onAssemblyRefreshStarted()
	{
		if (!isDestroyed())
			mEditorWidget->triggerOnDestroy();
	}

	MonoObject* ScriptEditorWindow::_createManagedInstance(bool construct)
	{
		if (!isDestroyed())
		{
			mEditorWidget->createManagedInstance();

			return mEditorWidget->getManagedInstance();
		}
		else
			return nullptr;
	}

	bool ScriptEditorWindow::internal_hasFocus(ScriptEditorWindow* thisPtr)
	{
		if (!thisPtr->isDestroyed())
			return thisPtr->getEditorWidget()->hasFocus();
		else
			return false;
	}

	void ScriptEditorWindow::internal_screenToWindowPos(ScriptEditorWindow* thisPtr, Vector2I screenPos, Vector2I* windowPos)
	{
		if (!thisPtr->isDestroyed())
			*windowPos = thisPtr->getEditorWidget()->screenToWidgetPos(screenPos);
		else
			*windowPos = screenPos;
	}

	void ScriptEditorWindow::internal_windowToScreenPos(ScriptEditorWindow* thisPtr, Vector2I windowPos, Vector2I* screenPos)
	{
		if (!thisPtr->isDestroyed())
			*screenPos = thisPtr->getEditorWidget()->widgetToScreenPos(windowPos);
		else
			*screenPos = windowPos;
	}

	UINT32 ScriptEditorWindow::internal_getWidth(ScriptEditorWindow* thisPtr)
	{
		if (!thisPtr->isDestroyed())
			return thisPtr->mEditorWidget->getWidth();
		else
			return 0;
	}

	UINT32 ScriptEditorWindow::internal_getHeight(ScriptEditorWindow* thisPtr)
	{
		if (!thisPtr->isDestroyed())
			return thisPtr->mEditorWidget->getHeight();
		else
			return 0;
	}

	void ScriptEditorWindow::onWidgetResized(UINT32 width, UINT32 height)
	{
		if (isDestroyed() || mManagedInstance == nullptr)
			return;

		void* params[2] = { &width, &height };
		onResizedMethod->invokeVirtual(mManagedInstance, params);
	}

	void ScriptEditorWindow::onFocusChanged(bool inFocus)
	{
		if (isDestroyed() || mManagedInstance == nullptr)
			return;

		void* params[1] = { &inFocus};
		onFocusChangedMethod->invokeVirtual(mManagedInstance, params);
	}

	void ScriptEditorWindow::registerManagedEditorWindows()
	{
		MonoAssembly* assembly = MonoManager::instance().getAssembly(EDITOR_ASSEMBLY);

		if(assembly != nullptr)
		{
			MonoClass* editorWindowClass = assembly->getClass("BansheeEditor", "EditorWindow");

			const Vector<MonoClass*>& allClasses = assembly->getAllClasses();
			for(auto& curClass : allClasses)
			{
				if(curClass->isSubClassOf(editorWindowClass) && curClass != editorWindowClass)
				{
					const String& className = curClass->getFullName();
					EditorWidgetManager::instance().registerWidget(className, 
						std::bind(&ScriptEditorWindow::openEditorWidgetCallback, curClass->getNamespace(), curClass->getTypeName(), std::placeholders::_1));
					AvailableWindowTypes.push_back(className);
				}
			}
		}
	}

	void ScriptEditorWindow::clearRegisteredEditorWindow()
	{
		for (auto& windowType : AvailableWindowTypes)
		{
			EditorWidgetManager::instance().unregisterWidget(windowType);
		}

		AvailableWindowTypes.clear();
	}

	EditorWidgetBase* ScriptEditorWindow::openEditorWidgetCallback(String ns, String type, EditorWidgetContainer& parentContainer)
	{
		ScriptEditorWidget* editorWidget = bs_new<ScriptEditorWidget>(ns, type, parentContainer);
		ScriptEditorWindow* nativeInstance = new (bs_alloc<ScriptEditorWindow>()) ScriptEditorWindow(editorWidget);

		ScriptEditorWindow::registerScriptEditorWindow(nativeInstance);

		mono_runtime_object_init(editorWidget->getManagedInstance()); // Construct it
		editorWidget->setScriptOwner(nativeInstance);
		editorWidget->triggerOnInitialize();

		return editorWidget;
	}

	void ScriptEditorWindow::registerScriptEditorWindow(ScriptEditorWindow* editorWindow)
	{
		if(editorWindow == nullptr)
			return;

		auto findIter = OpenScriptEditorWindows.find(editorWindow->mName);
		if(findIter == OpenScriptEditorWindows.end())
		{
			EditorWindowHandle newHandle;
			newHandle.nativeObj = editorWindow;
			newHandle.gcHandle = mono_gchandle_new(editorWindow->mManagedInstance, false);

			OpenScriptEditorWindows[editorWindow->mName] = newHandle;
		}
	}

	void ScriptEditorWindow::unregisterScriptEditorWindow(const String& windowTypeName)
	{
		auto findIter = OpenScriptEditorWindows.find(windowTypeName);
		if(findIter != OpenScriptEditorWindows.end())
		{
			EditorWindowHandle& foundHandle = findIter->second;
			mono_gchandle_free(foundHandle.gcHandle);

			OpenScriptEditorWindows.erase(findIter);
		}
	}

	ScriptEditorWidget::ScriptEditorWidget(const String& ns, const String& type, EditorWidgetContainer& parentContainer)
		:EditorWidgetBase(HString(toWString(type)), ns + "." + type, parentContainer), mNamespace(ns), mTypename(type),
		mUpdateThunk(nullptr), mManagedInstance(nullptr), mOnInitializeThunk(nullptr), mOnDestroyThunk(nullptr), 
		mContentsPanel(nullptr), mScriptOwner(nullptr)
	{
		createManagedInstance();
	}

	ScriptEditorWidget::~ScriptEditorWidget()
	{
		mScriptOwner->mIsDestroyed = true;
		mScriptOwner->mEditorWidget = nullptr;

		mContentsPanel->destroyChildren();
		mContentsPanel->markAsDestroyed();
		mContentsPanel = nullptr;

		triggerOnDestroy();
		ScriptEditorWindow::unregisterScriptEditorWindow(getName());
	}

	bool ScriptEditorWidget::createManagedInstance()
	{
		MonoAssembly* assembly = MonoManager::instance().getAssembly(EDITOR_ASSEMBLY);

		if (assembly != nullptr)
		{
			MonoClass* editorWindowClass = assembly->getClass(mNamespace, mTypename);

			if (editorWindowClass != nullptr)
			{
				mManagedInstance = editorWindowClass->createInstance();

				MonoObject* guiPanel = ScriptGUIPanel::createFromExisting(mContent);
				mContentsPanel = ScriptGUILayout::toNative(guiPanel);
				ScriptEditorWindow::guiPanelField->setValue(mManagedInstance, guiPanel);

				reloadMonoTypes(editorWindowClass);
				return true;
			}
		}

		return false;
	}

	void ScriptEditorWidget::triggerOnInitialize()
	{
		if (mOnInitializeThunk != nullptr && mManagedInstance != nullptr)
		{
			MonoException* exception = nullptr;

			// Note: Not calling virtual methods. Can be easily done if needed but for now doing this
			// for some extra speed.
			mOnInitializeThunk(mManagedInstance, &exception);

			MonoUtil::throwIfException(exception);
		}
	}

	void ScriptEditorWidget::triggerOnDestroy()
	{
		if (mOnDestroyThunk != nullptr && mManagedInstance != nullptr)
		{
			MonoException* exception = nullptr;

			// Note: Not calling virtual methods. Can be easily done if needed but for now doing this
			// for some extra speed.
			mOnDestroyThunk(mManagedInstance, &exception);

			MonoUtil::throwIfException(exception);
		}
	}

	void ScriptEditorWidget::update()
	{
		if (mUpdateThunk != nullptr && mManagedInstance != nullptr)
		{
			MonoException* exception = nullptr;

			// Note: Not calling virtual methods. Can be easily done if needed but for now doing this
			// for some extra speed.
			mUpdateThunk(mManagedInstance, &exception);

			MonoUtil::throwIfException(exception);
		}
	}

	void ScriptEditorWidget::reloadMonoTypes(MonoClass* windowClass)
	{
		MonoMethod* updateMethod = windowClass->getMethod("EditorUpdate", 0);

		if (updateMethod != nullptr)
			mUpdateThunk = (UpdateThunkDef)updateMethod->getThunk();

		MonoMethod* onInitializeMethod = windowClass->getMethod("OnInitialize", 0);

		if (onInitializeMethod != nullptr)
			mOnInitializeThunk = (OnInitializeThunkDef)onInitializeMethod->getThunk();

		MonoMethod* onDestroyMethod = windowClass->getMethod("OnDestroy", 0);

		if (onDestroyMethod != nullptr)
			mOnDestroyThunk = (OnDestroyThunkDef)onDestroyMethod->getThunk();
	}
}