#include "BsGUIFieldBase.h"
#include "BsGUILabel.h"
#include "BsGUILayoutX.h"
#include "BsGUIWidget.h"
#include "BsGUISkin.h"
#include "BsGUILayoutUtility.h"

namespace BansheeEngine
{
	const UINT32 GUIFieldBase::DEFAULT_LABEL_WIDTH = 100;

	GUIFieldBase::GUIFieldBase(const PrivatelyConstruct& dummy, const GUIContent& labelContent, UINT32 labelWidth,
		const String& style, const GUIDimensions& dimensions, bool withLabel)
		:GUIElementContainer(dimensions, style), mLabel(nullptr)
	{
		mLayout = GUILayoutX::create();
		_registerChildElement(mLayout);

		if(withLabel)
		{
			mLabel = GUILabel::create(labelContent, GUIOptions(GUIOption::fixedWidth(labelWidth)), getSubStyleName(getLabelStyleType()));
			mLayout->addElement(mLabel);
		}
	}

	void GUIFieldBase::_updateLayoutInternal(const GUILayoutData& data)
	{
		GUILayoutData childData = data;
		childData.clipRect.x -= data.area.x;
		childData.clipRect.y -= data.area.y;

		mLayout->_setLayoutData(childData);

		childData.clipRect = data.clipRect;

		mLayout->_updateLayoutInternal(childData);
	}

	Vector2I GUIFieldBase::_getOptimalSize() const
	{
		return GUILayoutUtility::calcOptimalSize(mLayout);
	}

	void GUIFieldBase::styleUpdated()
	{
		if (mLabel != nullptr)
			mLabel->setStyle(getSubStyleName(getLabelStyleType()));
	}
}