#include "BsGUIFieldBase.h"
#include "BsGUILabel.h"
#include "BsGUILayout.h"
#include "BsGUIWidget.h"
#include "BsGUISkin.h"
#include "BsGUILayoutUtility.h"

namespace BansheeEngine
{
	const UINT32 GUIFieldBase::DEFAULT_LABEL_WIDTH = 100;

	GUIFieldBase::GUIFieldBase(const PrivatelyConstruct& dummy, const GUIContent& labelContent, UINT32 labelWidth,
		const String& style, const GUILayoutOptions& layoutOptions, bool withLabel)
		:GUIElementContainer(layoutOptions, style), mLabel(nullptr)
	{
		mLayout = &addLayoutXInternal(this);

		if(withLabel)
		{
			mLabel = GUILabel::create(labelContent, GUIOptions(GUIOption::fixedWidth(labelWidth)), getSubStyleName(getLabelStyleType()));
			mLayout->addElement(mLabel);
		}
	}

	void GUIFieldBase::_updateLayoutInternal(INT32 x, INT32 y, UINT32 width, UINT32 height,
		Rect2I clipRect, UINT8 widgetDepth, UINT16 areaDepth)
	{
		mLayout->_updateLayoutInternal(x, y, width, height, clipRect, widgetDepth, areaDepth);
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