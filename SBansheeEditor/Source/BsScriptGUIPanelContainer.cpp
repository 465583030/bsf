#include "BsScriptGUIPanelContainer.h"
#include "BsGUIPanelContainer.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoMethod.h"
#include "BsGUIOptions.h"
#include "BsDebug.h"

namespace BansheeEngine
{
	ScriptGUIPanelContainer::ScriptGUIPanelContainer(MonoObject* instance, GUIPanelContainer* panelContainer)
		:TScriptGUIElement(instance, panelContainer)
	{

	}

	void ScriptGUIPanelContainer::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptGUIPanelContainer::internal_createInstance);
		metaData.scriptClass->addInternalCall("Internal_SetPanel", &ScriptGUIPanelContainer::internal_setPanel);
	}

	void ScriptGUIPanelContainer::internal_createInstance(MonoObject* instance, MonoObject* panel, MonoArray* guiOptions)
	{
		GUIOptions options;

		UINT32 arrayLen = (UINT32)mono_array_length(guiOptions);
		for (UINT32 i = 0; i < arrayLen; i++)
			options.addOption(mono_array_get(guiOptions, GUIOption, i));

		ScriptGUIPanel* guiPanel = ScriptGUIPanel::toNative(panel);
		LOGWRN("Creating panel: " + toString((UINT64)guiPanel));
		GUIPanelContainer* guiPanelContainer = GUIPanelContainer::create(*guiPanel, options);

		ScriptGUIPanelContainer* nativeInstance = new (bs_alloc<ScriptGUIPanelContainer>()) ScriptGUIPanelContainer(instance, guiPanelContainer);
	}

	void ScriptGUIPanelContainer::internal_setPanel(ScriptGUIPanelContainer* nativeInstance, MonoObject* panel)
	{
		ScriptGUIPanel* guiPanel = ScriptGUIPanel::toNative(panel);

		GUIPanelContainer* panelContainer = static_cast<GUIPanelContainer*>(nativeInstance->getGUIElement());
		panelContainer->setPanel(*guiPanel);
	}
}