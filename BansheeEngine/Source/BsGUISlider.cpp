#include "BsGUISlider.h"
#include "BsGUIWidget.h"
#include "BsGUISkin.h"
#include "BsGUISliderHandle.h"
#include "BsGUITexture.h"
#include "BsSpriteTexture.h"
#include "BsGUILayoutOptions.h"

using namespace std::placeholders;

namespace BansheeEngine
{
	GUISlider::GUISlider(bool horizontal, const String& styleName, const GUILayoutOptions& layoutOptions)
		:GUIElementContainer(layoutOptions, styleName)
	{
		mSliderHandle = GUISliderHandle::create(horizontal, getSubStyleName(getHandleStyleType()));
		mBackground = GUITexture::create(getSubStyleName(getBackgroundStyleType()));

		mBackground->_setElementDepth(1);

		_registerChildElement(mSliderHandle);
		_registerChildElement(mBackground);

		mHandleMovedConn = mSliderHandle->onHandleMoved.connect(std::bind(&GUISlider::onHandleMoved, this, _1));
	}

	GUISlider::~GUISlider()
	{
		mHandleMovedConn.disconnect();
	}

	const String& GUISlider::getHandleStyleType()
	{
		static String HANDLE_STYLE_TYPE = "SliderHandle";
		return HANDLE_STYLE_TYPE;
	}

	const String& GUISlider::getBackgroundStyleType()
	{
		static String BACKGROUND_STYLE_TYPE = "SliderBackground";
		return BACKGROUND_STYLE_TYPE;
	}

	Vector2I GUISlider::_getOptimalSize() const
	{
		Vector2I optimalSize = mSliderHandle->_getOptimalSize();

		Vector2I backgroundSize = mBackground->_getOptimalSize();
		optimalSize.x = std::max(optimalSize.x, backgroundSize.x);
		optimalSize.y = std::max(optimalSize.y, backgroundSize.y);

		return optimalSize;
	}

	void GUISlider::_updateLayoutInternal(INT32 x, INT32 y, UINT32 width, UINT32 height,
		Rect2I clipRect, UINT8 widgetDepth, UINT16 areaDepth)
	{
		Vector2I offset(x, y);
		Rect2I elemClipRect(clipRect.x - offset.x, clipRect.y - offset.y, clipRect.width, clipRect.height);

		mBackground->_setOffset(offset);
		mBackground->_setWidth(width);
		mBackground->_setHeight(height);
		mBackground->_setAreaDepth(areaDepth);
		mBackground->_setWidgetDepth(widgetDepth);
		mBackground->_setClipRect(elemClipRect);

		mSliderHandle->_setOffset(offset);
		mSliderHandle->_setWidth(width);
		mSliderHandle->_setHeight(height);
		mSliderHandle->_setAreaDepth(areaDepth);
		mSliderHandle->_setWidgetDepth(widgetDepth);
		mSliderHandle->_setClipRect(elemClipRect);
	}

	void GUISlider::setPercent(float pct)
	{
		mSliderHandle->setHandlePos(pct);
	}

	float GUISlider::getPercent() const
	{
		return mSliderHandle->getHandlePos();
	}

	void GUISlider::onHandleMoved(float newPosition)
	{
		onChanged(newPosition);
	}

	GUISliderHorz::GUISliderHorz(const String& styleName, const GUILayoutOptions& layoutOptions)
		:GUISlider(true, styleName, layoutOptions)
	{

	}

	GUISliderHorz* GUISliderHorz::create(const String& styleName)
	{
		return new (bs_alloc<GUISliderHorz, PoolAlloc>()) GUISliderHorz(getStyleName<GUISliderHorz>(styleName), GUILayoutOptions::create());
	}

	GUISliderHorz* GUISliderHorz::create(const GUIOptions& layoutOptions, const String& styleName)
	{
		return new (bs_alloc<GUISliderHorz, PoolAlloc>()) GUISliderHorz(getStyleName<GUISliderHorz>(styleName), GUILayoutOptions::create(layoutOptions));
	}

	const String& GUISliderHorz::getGUITypeName()
	{
		static String typeName = "SliderHorz";
		return typeName;
	}

	GUISliderVert::GUISliderVert(const String& styleName, const GUILayoutOptions& layoutOptions)
		:GUISlider(false, styleName, layoutOptions)
	{

	}

	GUISliderVert* GUISliderVert::create(const String& styleName)
	{
		return new (bs_alloc<GUISliderVert, PoolAlloc>()) GUISliderVert(getStyleName<GUISliderVert>(styleName), GUILayoutOptions::create());
	}

	GUISliderVert* GUISliderVert::create(const GUIOptions& layoutOptions, const String& styleName)
	{
		return new (bs_alloc<GUISliderVert, PoolAlloc>()) GUISliderVert(getStyleName<GUISliderVert>(styleName), GUILayoutOptions::create(layoutOptions));
	}

	const String& GUISliderVert::getGUITypeName()
	{
		static String typeName = "SliderVert";
		return typeName;
	}
}