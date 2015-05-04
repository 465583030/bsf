#include "BsGUISlider.h"
#include "BsGUIWidget.h"
#include "BsGUISkin.h"
#include "BsGUISliderHandle.h"
#include "BsGUITexture.h"
#include "BsSpriteTexture.h"
#include "BsGUIDimensions.h"

using namespace std::placeholders;

namespace BansheeEngine
{
	GUISlider::GUISlider(bool horizontal, const String& styleName, const GUIDimensions& dimensions)
		:GUIElementContainer(dimensions, styleName)
	{
		mSliderHandle = GUISliderHandle::create(horizontal, true, getSubStyleName(getHandleStyleType()));
		mBackground = GUITexture::create(getSubStyleName(getBackgroundStyleType()));

		mBackground->_setElementDepth(mSliderHandle->_getRenderElementDepthRange());

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
		Rect2I clipRect, UINT8 widgetDepth, INT16 panelDepth, UINT16 panelDepthRangeMin, UINT16 panelDepthRangeMax)
	{
		Vector2I offset(x, y);
		Rect2I elemClipRect(clipRect.x - offset.x, clipRect.y - offset.y, clipRect.width, clipRect.height);

		mBackground->_setPosition(offset);
		mBackground->_setWidth(width);
		mBackground->_setHeight(height);
		mBackground->_setAreaDepth(panelDepth);
		mBackground->_setWidgetDepth(widgetDepth);
		mBackground->_setClipRect(elemClipRect);

		mSliderHandle->_setPosition(offset);
		mSliderHandle->_setWidth(width);
		mSliderHandle->_setHeight(height);
		mSliderHandle->_setAreaDepth(panelDepth);
		mSliderHandle->_setWidgetDepth(widgetDepth);
		mSliderHandle->_setClipRect(elemClipRect);
	}

	void GUISlider::styleUpdated()
	{
		mBackground->setStyle(getSubStyleName(getBackgroundStyleType()));
		mSliderHandle->setStyle(getSubStyleName(getHandleStyleType()));
	}

	void GUISlider::setPercent(float pct)
	{
		mSliderHandle->setHandlePos(pct);
	}

	float GUISlider::getPercent() const
	{
		return mSliderHandle->getHandlePos();
	}

	void GUISlider::setTint(const Color& color)
	{
		mBackground->setTint(color);
		mSliderHandle->setTint(color);
	}

	void GUISlider::onHandleMoved(float newPosition)
	{
		onChanged(newPosition);
	}

	GUISliderHorz::GUISliderHorz(const String& styleName, const GUIDimensions& dimensions)
		:GUISlider(true, styleName, dimensions)
	{

	}

	GUISliderHorz* GUISliderHorz::create(const String& styleName)
	{
		return new (bs_alloc<GUISliderHorz, PoolAlloc>()) GUISliderHorz(getStyleName<GUISliderHorz>(styleName), GUIDimensions::create());
	}

	GUISliderHorz* GUISliderHorz::create(const GUIOptions& options, const String& styleName)
	{
		return new (bs_alloc<GUISliderHorz, PoolAlloc>()) GUISliderHorz(getStyleName<GUISliderHorz>(styleName), GUIDimensions::create(options));
	}

	const String& GUISliderHorz::getGUITypeName()
	{
		static String typeName = "SliderHorz";
		return typeName;
	}

	GUISliderVert::GUISliderVert(const String& styleName, const GUIDimensions& dimensions)
		:GUISlider(false, styleName, dimensions)
	{

	}

	GUISliderVert* GUISliderVert::create(const String& styleName)
	{
		return new (bs_alloc<GUISliderVert, PoolAlloc>()) GUISliderVert(getStyleName<GUISliderVert>(styleName), GUIDimensions::create());
	}

	GUISliderVert* GUISliderVert::create(const GUIOptions& options, const String& styleName)
	{
		return new (bs_alloc<GUISliderVert, PoolAlloc>()) GUISliderVert(getStyleName<GUISliderVert>(styleName), GUIDimensions::create(options));
	}

	const String& GUISliderVert::getGUITypeName()
	{
		static String typeName = "SliderVert";
		return typeName;
	}
}