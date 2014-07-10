//__________________________ Banshee Project - A modern game development toolkit _________________________________//
//_____________________________________ www.banshee-project.com __________________________________________________//
//________________________ Copyright (c) 2014 Marko Pintera. All rights reserved. ________________________________//
#include "BsGUIToggleGroup.h"
#include "BsGUIToggle.h"

namespace BansheeEngine
{
	GUIToggleGroup::~GUIToggleGroup()
	{
		for(auto& button : mButtons)
		{
			button->_setToggleGroup(nullptr);
		}
	}

	void GUIToggleGroup::initialize(const std::shared_ptr<GUIToggleGroup>& sharedPtr)
	{
		mThis = sharedPtr;
	}

	void GUIToggleGroup::_add(GUIToggle* toggle)
	{
		auto iterFind = std::find(begin(mButtons), end(mButtons), toggle);
		if(iterFind != end(mButtons))
			return;

		mButtons.push_back(toggle);
		toggle->_setToggleGroup(mThis.lock());
	}

	void GUIToggleGroup::_remove(GUIToggle* toggle)
	{
		auto sharedPtr = mThis.lock(); // Make sure we keep a reference because calling _setToggleGroup(nullptr) 
		                               // may otherwise clear the last reference and cause us to destruct

		auto iterFind = std::find(begin(mButtons), end(mButtons), toggle);
		if(iterFind == end(mButtons))
			return;

		(*iterFind)->_setToggleGroup(nullptr);
		mButtons.erase(iterFind);
	}
}