#include "BsScriptContextMenu.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoMethod.h"
#include "BsMonoUtil.h"
#include "BsGUIContextMenu.h"
#include "BsScriptHString.h"

using namespace std::placeholders;

namespace BansheeEngine
{
	ScriptContextMenu::OnEntryTriggeredThunkDef ScriptContextMenu::onEntryTriggered;

	ScriptContextMenu::ScriptContextMenu(MonoObject* instance)
		: ScriptObject(instance)
	{
		mContextMenu = bs_shared_ptr<GUIContextMenu>();
	}

	void ScriptContextMenu::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptContextMenu::internal_CreateInstance);
		metaData.scriptClass->addInternalCall("Internal_AddItem", &ScriptContextMenu::internal_AddItem);
		metaData.scriptClass->addInternalCall("Internal_AddSeparator", &ScriptContextMenu::internal_AddSeparator);
		metaData.scriptClass->addInternalCall("Internal_SetLocalizedName", &ScriptContextMenu::internal_SetLocalizedName);

		onEntryTriggered = (OnEntryTriggeredThunkDef)metaData.scriptClass->getMethod("InternalDoOnEntryTriggered", 1)->getThunk();
	}

	void ScriptContextMenu::internal_CreateInstance(MonoObject* instance)
	{
		ScriptContextMenu* nativeInstance = new (bs_alloc<ScriptContextMenu>()) ScriptContextMenu(instance);
	}

	void ScriptContextMenu::internal_AddItem(ScriptContextMenu* instance, MonoString* path, UINT32 callbackIdx,
		ShortcutKey shortcut)
	{
		WString nativePath = MonoUtil::monoToWString(path);

		GUIContextMenuPtr contextMenu = instance->getInternal();
		contextMenu->addMenuItem(nativePath, std::bind(&ScriptContextMenu::onContextMenuItemTriggered,
			instance, callbackIdx), 0, shortcut);
	}

	void ScriptContextMenu::internal_AddSeparator(ScriptContextMenu* instance, MonoString* path)
	{
		WString nativePath = MonoUtil::monoToWString(path);

		GUIContextMenuPtr contextMenu = instance->getInternal();
		contextMenu->addSeparator(nativePath, 0);
	}

	void ScriptContextMenu::internal_SetLocalizedName(ScriptContextMenu* instance, MonoString* label, ScriptHString* name)
	{
		if (label == nullptr || name == nullptr)
			return;

		WString nativeLabel = MonoUtil::monoToWString(label);
		GUIContextMenuPtr contextMenu = instance->getInternal();
		contextMenu->setLocalizedName(nativeLabel, name->getInternalValue());
	}

	void ScriptContextMenu::onContextMenuItemTriggered(UINT32 idx)
	{
		MonoException* exception = nullptr;
		onEntryTriggered(getManagedInstance(), idx, &exception);

		MonoUtil::throwIfException(exception);
	}
}