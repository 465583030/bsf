#include "BsScriptGUIVector2Field.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoMethod.h"
#include "BsSpriteTexture.h"
#include "BsMonoUtil.h"
#include "BsGUILayout.h"
#include "BsGUIVector2Field.h"
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
	ScriptGUIVector2Field::OnChangedThunkDef ScriptGUIVector2Field::onChangedThunk;

	ScriptGUIVector2Field::ScriptGUIVector2Field(MonoObject* instance, GUIVector2Field* vector2Field)
		:TScriptGUIElement(instance, vector2Field)
	{

	}

	void ScriptGUIVector2Field::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptGUIVector2Field::internal_createInstance);
		metaData.scriptClass->addInternalCall("Internal_GetValue", &ScriptGUIVector2Field::internal_getValue);
		metaData.scriptClass->addInternalCall("Internal_SetValue", &ScriptGUIVector2Field::internal_setValue);
		metaData.scriptClass->addInternalCall("Internal_HasInputFocus", &ScriptGUIVector2Field::internal_hasInputFocus);

		onChangedThunk = (OnChangedThunkDef)metaData.scriptClass->getMethod("DoOnChanged", 1)->getThunk();
	}

	void ScriptGUIVector2Field::internal_createInstance(MonoObject* instance, MonoObject* title, UINT32 titleWidth,
		MonoString* style, MonoArray* guiOptions, bool withTitle)
	{
		GUIOptions options;

		UINT32 arrayLen = (UINT32)mono_array_length(guiOptions);
		for (UINT32 i = 0; i < arrayLen; i++)
			options.addOption(mono_array_get(guiOptions, GUIOption, i));

		String styleName = toString(MonoUtil::monoToWString(style));

		GUIVector2Field* field = nullptr;
		if (withTitle)
		{
			GUIContent nativeContent(ScriptGUIContent::getText(title), ScriptGUIContent::getImage(title), ScriptGUIContent::getTooltip(title));
			field = GUIVector2Field::create(nativeContent, titleWidth, options, styleName);
		}
		else
		{
			field = GUIVector2Field::create(options, styleName);
		}

		field->onValueChanged.connect(std::bind(&ScriptGUIVector2Field::onChanged, instance, _1));

		ScriptGUIVector2Field* nativeInstance = new (bs_alloc<ScriptGUIVector2Field>()) ScriptGUIVector2Field(instance, field);
	}

	void ScriptGUIVector2Field::internal_getValue(ScriptGUIVector2Field* nativeInstance, Vector2* output)
	{
		GUIVector2Field* field = static_cast<GUIVector2Field*>(nativeInstance->getGUIElement());

		*output = field->getValue();
	}

	void ScriptGUIVector2Field::internal_setValue(ScriptGUIVector2Field* nativeInstance, Vector2 value)
	{
		GUIVector2Field* field = static_cast<GUIVector2Field*>(nativeInstance->getGUIElement());

		return field->setValue(value);
	}

	void ScriptGUIVector2Field::internal_hasInputFocus(ScriptGUIVector2Field* nativeInstance, bool* output)
	{
		GUIVector2Field* field = static_cast<GUIVector2Field*>(nativeInstance->getGUIElement());

		*output = field->hasInputFocus();
	}

	void ScriptGUIVector2Field::onChanged(MonoObject* instance, Vector2 newValue)
	{
		MonoException* exception = nullptr;
		onChangedThunk(instance, newValue, &exception);

		MonoUtil::throwIfException(exception);
	}
}