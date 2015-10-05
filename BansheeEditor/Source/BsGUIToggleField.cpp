#include "BsGUIToggleField.h"
#include "BsGUILayout.h"
#include "BsGUILabel.h"
#include "BsGUIToggle.h"
#include "BsBuiltinResources.h"
#include "BsCGUIWidget.h"
#include "BsGUIMouseEvent.h"
#include "BsCGUIWidget.h"
#include "BsCmdInputFieldValueChange.h"

using namespace std::placeholders;

namespace BansheeEngine
{
	GUIToggleField::GUIToggleField(const PrivatelyConstruct& dummy, const GUIContent& labelContent, 
		UINT32 labelWidth, const String& style, const GUIDimensions& dimensions, bool withLabel)
		:TGUIField(dummy, labelContent, labelWidth, style, dimensions, withLabel), mToggle(nullptr), mValue(false)
	{
		mToggle = GUIToggle::create(HString(L""), getSubStyleName(getToggleStyleType()));
		mToggle->onToggled.connect(std::bind(&GUIToggleField::valueChanged, this, _1));

		mLayout->addElement(mToggle);
	}

	void GUIToggleField::setValue(bool value)
	{
		if (mValue == value)
			return;

		mValue = value;

		if(value) 
			mToggle->toggleOn();
		else
			mToggle->toggleOff();
	}

	void GUIToggleField::setTint(const Color& color)
	{
		if (mLabel != nullptr)
			mLabel->setTint(color);

		mToggle->setTint(color);
	}

	void GUIToggleField::styleUpdated()
	{
		if (mLabel != nullptr)
			mLabel->setStyle(getSubStyleName(getLabelStyleType()));

		mToggle->setStyle(getSubStyleName(getToggleStyleType()));
	}

	void GUIToggleField::valueChanged(bool newValue)
	{
		CmdInputFieldValueChange<GUIToggleField, bool>::execute(this, newValue);

		if (!onValueChanged.empty())
			onValueChanged(newValue);
	}

	const String& GUIToggleField::getGUITypeName()
	{
		static String typeName = "GUIToggleField";
		return typeName;
	}

	const String& GUIToggleField::getToggleStyleType()
	{
		static String STYLE_TYPE = "EditorFieldToggleInput";
		return STYLE_TYPE;
	}
}