#include "BsGUIWindowDropArea.h"
#include "BsCoreApplication.h"
#include "BsTexture.h"
#include "BsGUIWidget.h"
#include "BsGUISkin.h"
#include "BsSpriteTexture.h"
#include "BsGUIDimensions.h"
#include "BsGUIMouseEvent.h"

namespace BansheeEngine
{
	const String& GUIWindowDropArea::getGUITypeName()
	{
		static String name = "WindowDropArea";
		return name;
	}

	GUIWindowDropArea::GUIWindowDropArea(const String& styleName, const GUIDimensions& layoutOptions)
		:GUITexture(styleName, HSpriteTexture(), GUIImageScaleMode::ScaleToFit, true, layoutOptions)
	{ }

	GUIWindowDropArea::~GUIWindowDropArea()
	{ }

	GUIWindowDropArea* GUIWindowDropArea::create(const String& styleName)
	{
		return new (bs_alloc<GUIWindowDropArea, PoolAlloc>()) 
			GUIWindowDropArea(getStyleName<GUIWindowDropArea>(styleName), GUIDimensions::create());
	}

	GUIWindowDropArea* GUIWindowDropArea::create(const GUIOptions& layoutOptions, const String& styleName)
	{
		return new (bs_alloc<GUIWindowDropArea, PoolAlloc>()) 
			GUIWindowDropArea(getStyleName<GUIWindowDropArea>(styleName), GUIDimensions::create(layoutOptions));
	}

	void GUIWindowDropArea::setFocused(bool focused)
	{
		if(focused)
			mActiveTexture = _getStyle()->focused.texture;
		else
			mActiveTexture = _getStyle()->normal.texture;

		markContentAsDirty();
	}

	bool GUIWindowDropArea::_mouseEvent(const GUIMouseEvent& ev)
	{
		if(ev.getType() == GUIMouseEventType::MouseDragAndDropDropped)
		{
			if(!onDraggedItemDropped.empty())
				onDraggedItemDropped();

			return true;
		}

		return false;
	}
}