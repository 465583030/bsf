#include "BsGUIDropDownHitBox.h"
#include "BsGUICommandEvent.h"
#include "BsGUIMouseEvent.h"
#include "BsGUIWidget.h"
#include "BsGUISkin.h"

namespace BansheeEngine
{
	const String& GUIDropDownHitBox::getGUITypeName()
	{
		static String name = "DropDownHitBox";
		return name;
	}

	GUIDropDownHitBox* GUIDropDownHitBox::create(bool captureMouse)
	{
		return new (bs_alloc<GUIDropDownHitBox, PoolAlloc>()) GUIDropDownHitBox(captureMouse, GUIDimensions::create());
	}

	GUIDropDownHitBox* GUIDropDownHitBox::create(bool captureMouse, const GUIOptions& options)
	{
		return new (bs_alloc<GUIDropDownHitBox, PoolAlloc>()) GUIDropDownHitBox(captureMouse, GUIDimensions::create(options));
	}

	GUIDropDownHitBox::GUIDropDownHitBox(bool captureMouse, const GUIDimensions& dimensions)
		:GUIElementContainer(dimensions), mCaptureMouse(captureMouse)
	{

	}

	bool GUIDropDownHitBox::_commandEvent(const GUICommandEvent& ev)
	{
		bool processed = GUIElementContainer::_commandEvent(ev);

		if(ev.getType() == GUICommandEventType::FocusGained)
		{
			if(!onFocusGained.empty())
				onFocusGained();

			return true;
		}
		else if(ev.getType() == GUICommandEventType::FocusLost)
		{
			if(!onFocusLost.empty())
				onFocusLost();

			return true;
		}

		return processed;
	}

	bool GUIDropDownHitBox::_mouseEvent(const GUIMouseEvent& ev)
	{
		bool processed = GUIElementContainer::_mouseEvent(ev);

		if(mCaptureMouse)
		{
			if(ev.getType() == GUIMouseEventType::MouseUp)
			{
				return true;
			}
			else if(ev.getType() == GUIMouseEventType::MouseDown)
			{
				return true;
			}
			else if (ev.getType() == GUIMouseEventType::MouseOver)
			{
				return true;
			}
			else if (ev.getType() == GUIMouseEventType::MouseOut)
			{
				return true;
			}
			else if (ev.getType() == GUIMouseEventType::MouseMove)
			{
				return true;
			}
			else if (ev.getType() == GUIMouseEventType::MouseDoubleClick)
			{
				return true;
			}
		}

		return processed;
	}

	bool GUIDropDownHitBox::_isInBounds(const Vector2I position) const
	{
		for(auto& bound : mBounds)
		{
			if(bound.contains(position))
				return true;
		}

		return false;
	}
};