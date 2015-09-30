#include "BsScriptGUITextureField.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoMethod.h"
#include "BsSpriteTexture.h"
#include "BsMonoUtil.h"
#include "BsGUILayout.h"
#include "BsGUITextureField.h"
#include "BsGUIOptions.h"
#include "BsGUIContent.h"
#include "BsScriptGUIElementStyle.h"
#include "BsScriptGUILayout.h"
#include "BsScriptHString.h"
#include "BsScriptGUIContent.h"
#include "BsScriptTexture.h"
#include "BsScriptResourceManager.h"

using namespace std::placeholders;

namespace BansheeEngine
{
	ScriptGUITextureField::OnChangedThunkDef ScriptGUITextureField::onChangedThunk;

	ScriptGUITextureField::ScriptGUITextureField(MonoObject* instance, GUITextureField* resourceField)
		:TScriptGUIElement(instance, resourceField)
	{

	}

	void ScriptGUITextureField::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptGUITextureField::internal_createInstance);
		metaData.scriptClass->addInternalCall("Internal_GetValue", &ScriptGUITextureField::internal_getValue);
		metaData.scriptClass->addInternalCall("Internal_SetValue", &ScriptGUITextureField::internal_setValue);
		metaData.scriptClass->addInternalCall("Internal_SetTint", &ScriptGUITextureField::internal_setTint);

		onChangedThunk = (OnChangedThunkDef)metaData.scriptClass->getMethod("Internal_DoOnChanged", 1)->getThunk();
	}

	void ScriptGUITextureField::internal_createInstance(MonoObject* instance, MonoObject* title, UINT32 titleWidth,
		MonoString* style, MonoArray* guiOptions, bool withTitle)
	{
		GUIOptions options;

		UINT32 arrayLen = (UINT32)mono_array_length(guiOptions);
		for (UINT32 i = 0; i < arrayLen; i++)
			options.addOption(mono_array_get(guiOptions, GUIOption, i));

		String styleName = toString(MonoUtil::monoToWString(style));

		GUITextureField* guiTextureField = nullptr;
		if (withTitle)
		{
			GUIContent nativeContent(ScriptGUIContent::getText(title), ScriptGUIContent::getImage(title), ScriptGUIContent::getTooltip(title));
			guiTextureField = GUITextureField::create(nativeContent, titleWidth, options, styleName);
		}
		else
		{
			guiTextureField = GUITextureField::create(options, styleName);
		}

		guiTextureField->onValueChanged.connect(std::bind(&ScriptGUITextureField::onChanged, instance, _1));

		ScriptGUITextureField* nativeInstance = new (bs_alloc<ScriptGUITextureField>()) ScriptGUITextureField(instance, guiTextureField);
	}

	void ScriptGUITextureField::internal_getValue(ScriptGUITextureField* nativeInstance, MonoObject** output)
	{
		GUITextureField* textureField = static_cast<GUITextureField*>(nativeInstance->getGUIElement());

		HTexture resource = textureField->getValue();
		*output = nativeToManagedResource(resource);
	}

	void ScriptGUITextureField::internal_setValue(ScriptGUITextureField* nativeInstance, MonoObject* value)
	{
		GUITextureField* textureField = static_cast<GUITextureField*>(nativeInstance->getGUIElement());

		if (value == nullptr)
			textureField->setValue(HTexture());
		else
		{
			ScriptTextureBase* scriptTexture = ScriptTexture::toNative(value);
			textureField->setValue(static_resource_cast<Texture>(scriptTexture->getGenericHandle()));
		}
	}

	void ScriptGUITextureField::internal_setTint(ScriptGUITextureField* nativeInstance, Color color)
	{
		GUITextureField* textureField = (GUITextureField*)nativeInstance->getGUIElement();
		textureField->setTint(color);
	}

	void ScriptGUITextureField::onChanged(MonoObject* instance, const HTexture& newValue)
	{
		MonoObject* managedObj = nativeToManagedResource(newValue);
		MonoUtil::invokeThunk(onChangedThunk, instance, managedObj);
	}

	MonoObject* ScriptGUITextureField::nativeToManagedResource(const HTexture& instance)
	{
		if (instance == nullptr)
		{
			return nullptr;
		}
		else
		{
			ScriptResourceBase* scriptResource = ScriptResourceManager::instance().getScriptResource(instance.getUUID());
			if (scriptResource == nullptr)
				return nullptr;
			else
				return scriptResource->getManagedInstance();
		}
	}
}