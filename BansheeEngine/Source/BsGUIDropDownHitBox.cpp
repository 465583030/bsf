//__________________________ Banshee Project - A modern game development toolkit _________________________________//
//_____________________________________ www.banshee-project.com __________________________________________________//
//________________________ Copyright (c) 2014 Marko Pintera. All rights reserved. ________________________________//
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
		return new (bs_alloc<GUIDropDownHitBox, PoolAlloc>()) GUIDropDownHitBox(captureMouse, GUILayoutOptions::create());
	}

	GUIDropDownHitBox* GUIDropDownHitBox::create(bool captureMouse, const GUIOptions& layoutOptions)
	{
		return new (bs_alloc<GUIDropDownHitBox, PoolAlloc>()) GUIDropDownHitBox(captureMouse, GUILayoutOptions::create(layoutOptions));
	}

	GUIDropDownHitBox::GUIDropDownHitBox(bool captureMouse, const GUILayoutOptions& layoutOptions)
		:GUIElementContainer(layoutOptions), mCaptureMouse(captureMouse)
	{

	}

	bool GUIDropDownHitBox::commandEvent(const GUICommandEvent& ev)
	{
		bool processed = GUIElementContainer::commandEvent(ev);

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

	bool GUIDropDownHitBox::mouseEvent(const GUIMouseEvent& ev)
	{
		bool processed = GUIElementContainer::mouseEvent(ev);

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