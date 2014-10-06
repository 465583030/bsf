#include "BsScriptGUIFloatField.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoMethod.h"
#include "BsSpriteTexture.h"
#include "BsMonoUtil.h"
#include "BsGUILayout.h"
#include "BsGUIFloatField.h"
#include "BsGUIOptions.h"
#include "BsGUIContent.h"
#include "BsScriptGUIElementStyle.h"
#include "BsScriptGUILayout.h"
#include "BsScriptGUIArea.h"
#include "BsScriptHString.h"
#include "BsScriptGUIContent.h"

using namespace std::placeholders;

namespace BansheeEngine
{
	ScriptGUIFloatField::OnChangedThunkDef ScriptGUIFloatField::onChangedThunk;

	ScriptGUIFloatField::ScriptGUIFloatField(MonoObject* instance, GUIFloatField* floatField)
		:TScriptGUIElement(instance, floatField)
	{

	}

	void ScriptGUIFloatField::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptGUIFloatField::internal_createInstance);
		metaData.scriptClass->addInternalCall("Internal_GetValue", &ScriptGUIFloatField::internal_getValue);
		metaData.scriptClass->addInternalCall("Internal_SetValue", &ScriptGUIFloatField::internal_setValue);
		metaData.scriptClass->addInternalCall("Internal_HasInputFocus", &ScriptGUIFloatField::internal_hasInputFocus);

		onChangedThunk = (OnChangedThunkDef)metaData.scriptClass->getMethod("DoOnChanged", 1)->getThunk();
	}

	void ScriptGUIFloatField::internal_createInstance(MonoObject* instance, MonoObject* title, UINT32 titleWidth,
		MonoString* style, MonoArray* guiOptions, bool withTitle)
	{
		GUIOptions options;

		UINT32 arrayLen = (UINT32)mono_array_length(guiOptions);
		for (UINT32 i = 0; i < arrayLen; i++)
			options.addOption(mono_array_get(guiOptions, GUIOption, i));

		String styleName = toString(MonoUtil::monoToWString(style));

		GUIFloatField* guiFloatField = nullptr;
		if (withTitle)
		{
			GUIContent nativeContent(ScriptGUIContent::getText(title), ScriptGUIContent::getImage(title), ScriptGUIContent::getTooltip(title));
			guiFloatField = GUIFloatField::create(nativeContent, titleWidth, options, styleName);
		}
		else
		{
			guiFloatField = GUIFloatField::create(options, styleName);
		}

		guiFloatField->onValueChanged.connect(std::bind(&ScriptGUIFloatField::onChanged, instance, _1));

		ScriptGUIFloatField* nativeInstance = new (bs_alloc<ScriptGUIFloatField>()) ScriptGUIFloatField(instance, guiFloatField);
	}

	void ScriptGUIFloatField::internal_getValue(ScriptGUIFloatField* nativeInstance, float* output)
	{
		GUIFloatField* floatField = static_cast<GUIFloatField*>(nativeInstance->getGUIElement());

		*output = floatField->getValue();
	}

	void ScriptGUIFloatField::internal_setValue(ScriptGUIFloatField* nativeInstance, float value)
	{
		GUIFloatField* floatField = static_cast<GUIFloatField*>(nativeInstance->getGUIElement());

		return floatField->setValue(value);
	}

	void ScriptGUIFloatField::internal_hasInputFocus(ScriptGUIFloatField* nativeInstance, bool* output)
	{
		GUIFloatField* floatField = static_cast<GUIFloatField*>(nativeInstance->getGUIElement());

		*output = floatField->hasInputFocus();
	}

	void ScriptGUIFloatField::onChanged(MonoObject* instance, float newValue)
	{
		MonoException* exception = nullptr;
		onChangedThunk(instance, newValue, &exception);

		MonoUtil::throwIfException(exception);
	}
}