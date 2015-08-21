#include "BsScriptGUIContent.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoUtil.h"
#include "BsScriptHString.h"
#include "BsScriptSpriteTexture.h"
#include "BsSpriteTexture.h"

namespace BansheeEngine
{
	MonoField* ScriptGUIContent::mTextField;
	MonoField* ScriptGUIContent::mTooltipField;
	MonoField* ScriptGUIContent::mImageField;

	ScriptGUIContent::ScriptGUIContent(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptGUIContent::initRuntimeData()
	{
		mTextField = metaData.scriptClass->getField("text");
		mTooltipField = metaData.scriptClass->getField("tooltip");
		mImageField = metaData.scriptClass->getField("image");
	}

	const HString& ScriptGUIContent::getText(MonoObject* instance)
	{
		MonoObject* textManaged = nullptr;
		mTextField->getValue(instance, &textManaged);

		if(textManaged == nullptr)
			return HString::dummy();

		ScriptHString* textScript = ScriptHString::toNative(textManaged);
		return textScript->getInternalValue();
	}

	const HString& ScriptGUIContent::getTooltip(MonoObject* instance)
	{
		MonoObject* tooltipManaged = nullptr;
		mTooltipField->getValue(instance, &tooltipManaged);

		if(tooltipManaged == nullptr)
			return HString::dummy();

		ScriptHString* tooltipScript = ScriptHString::toNative(tooltipManaged);
		return tooltipScript->getInternalValue();
	}

	HSpriteTexture ScriptGUIContent::getImage(MonoObject* instance)
	{
		MonoObject* imageManaged = nullptr;
		mImageField->getValue(instance, &imageManaged);

		if(imageManaged == nullptr)
			return HSpriteTexture();

		ScriptSpriteTexture* imageScript = ScriptSpriteTexture::toNative(imageManaged);
		return imageScript->getInternalValue();
	}
}