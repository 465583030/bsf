#include "BsScriptGUISlider.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoMethod.h"
#include "BsSpriteTexture.h"
#include "BsMonoUtil.h"
#include "BsGUILayout.h"
#include "BsGUISlider.h"
#include "BsGUIOptions.h"
#include "BsScriptGUIElementStyle.h"
#include "BsScriptGUILayout.h"
#include "BsScriptGUIArea.h"
#include "BsScriptHString.h"
#include "BsScriptGUIContent.h"

using namespace std::placeholders;

namespace BansheeEngine
{
	ScriptGUISliderH::OnChangedThunkDef ScriptGUISliderH::onChangedThunk;

	ScriptGUISliderH::ScriptGUISliderH(MonoObject* instance, GUISliderHorz* slider)
		:TScriptGUIElement(instance, slider)
	{

	}

	void ScriptGUISliderH::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptGUISliderH::internal_createInstance);
		metaData.scriptClass->addInternalCall("Internal_SetPercent", &ScriptGUISliderH::internal_setPercent);
		metaData.scriptClass->addInternalCall("Internal_GetPercent", &ScriptGUISliderH::internal_getPercent);
		metaData.scriptClass->addInternalCall("Internal_SetTint", &ScriptGUISliderH::internal_setTint);

		onChangedThunk = (OnChangedThunkDef)metaData.scriptClass->getMethod("DoOnChanged", 1)->getThunk();
	}

	void ScriptGUISliderH::internal_createInstance(MonoObject* instance, MonoString* style, MonoArray* guiOptions)
	{
		GUIOptions options;

		UINT32 arrayLen = (UINT32)mono_array_length(guiOptions);
		for (UINT32 i = 0; i < arrayLen; i++)
			options.addOption(mono_array_get(guiOptions, GUIOption, i));

		GUISliderHorz* guiSlider = GUISliderHorz::create(options, toString(MonoUtil::monoToWString(style)));
		guiSlider->onChanged.connect(std::bind(&ScriptGUISliderH::onChanged, instance, _1));

		ScriptGUISliderH* nativeInstance = new (bs_alloc<ScriptGUISliderH>()) ScriptGUISliderH(instance, guiSlider);
	}

	void ScriptGUISliderH::internal_setPercent(ScriptGUISliderH* nativeInstance, float percent)
	{
		GUISliderHorz* slider = (GUISliderHorz*)nativeInstance->getGUIElement();
		slider->setPercent(percent);
	}

	float ScriptGUISliderH::internal_getPercent(ScriptGUISliderH* nativeInstance)
	{
		GUISliderHorz* slider = (GUISliderHorz*)nativeInstance->getGUIElement();
		return slider->getPercent();
	}

	void ScriptGUISliderH::internal_setTint(ScriptGUISliderH* nativeInstance, Color color)
	{
		GUISliderHorz* slider = (GUISliderHorz*)nativeInstance->getGUIElement();
		slider->setTint(color);
	}

	void ScriptGUISliderH::onChanged(MonoObject* instance, float percent)
	{
		MonoException* exception = nullptr;
		onChangedThunk(instance, percent, &exception);

		MonoUtil::throwIfException(exception);
	}

	ScriptGUISliderV::OnChangedThunkDef ScriptGUISliderV::onChangedThunk;

	ScriptGUISliderV::ScriptGUISliderV(MonoObject* instance, GUISliderVert* slider)
		:TScriptGUIElement(instance, slider)
	{

	}

	void ScriptGUISliderV::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptGUISliderV::internal_createInstance);
		metaData.scriptClass->addInternalCall("Internal_SetPercent", &ScriptGUISliderV::internal_setPercent);
		metaData.scriptClass->addInternalCall("Internal_GetPercent", &ScriptGUISliderV::internal_getPercent);
		metaData.scriptClass->addInternalCall("Internal_SetTint", &ScriptGUISliderV::internal_setTint);

		onChangedThunk = (OnChangedThunkDef)metaData.scriptClass->getMethod("DoOnChanged", 1)->getThunk();
	}

	void ScriptGUISliderV::internal_createInstance(MonoObject* instance, MonoString* style, MonoArray* guiOptions)
	{
		GUIOptions options;

		UINT32 arrayLen = (UINT32)mono_array_length(guiOptions);
		for (UINT32 i = 0; i < arrayLen; i++)
			options.addOption(mono_array_get(guiOptions, GUIOption, i));

		GUISliderVert* guiSlider = GUISliderVert::create(options, toString(MonoUtil::monoToWString(style)));
		guiSlider->onChanged.connect(std::bind(&ScriptGUISliderV::onChanged, instance, _1));

		ScriptGUISliderV* nativeInstance = new (bs_alloc<ScriptGUISliderV>()) ScriptGUISliderV(instance, guiSlider);
	}

	void ScriptGUISliderV::internal_setPercent(ScriptGUISliderV* nativeInstance, float percent)
	{
		GUISliderVert* slider = (GUISliderVert*)nativeInstance->getGUIElement();
		slider->setPercent(percent);
	}

	float ScriptGUISliderV::internal_getPercent(ScriptGUISliderV* nativeInstance)
	{
		GUISliderVert* slider = (GUISliderVert*)nativeInstance->getGUIElement();
		return slider->getPercent();
	}

	void ScriptGUISliderV::internal_setTint(ScriptGUISliderV* nativeInstance, Color color)
	{
		GUISliderVert* slider = (GUISliderVert*)nativeInstance->getGUIElement();
		slider->setTint(color);
	}

	void ScriptGUISliderV::onChanged(MonoObject* instance, float percent)
	{
		MonoException* exception = nullptr;
		onChangedThunk(instance, percent, &exception);

		MonoUtil::throwIfException(exception);
	}
}