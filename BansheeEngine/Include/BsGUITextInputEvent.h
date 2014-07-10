//__________________________ Banshee Project - A modern game development toolkit _________________________________//
//_____________________________________ www.banshee-project.com __________________________________________________//
//________________________ Copyright (c) 2014 Marko Pintera. All rights reserved. ________________________________//
#pragma once

#include "BsPrerequisites.h"
#include "BsInputFwd.h"
#include "BsVector2I.h"

namespace BansheeEngine
{
	/**
	 * @brief	A text input event representing input of a single character.
	 */
	class BS_EXPORT GUITextInputEvent
	{
	public:
		GUITextInputEvent();

		/**
		 * @brief	Character code that was input.
		 */
		const UINT32& getInputChar() const { return mInputChar; }
	private:
		friend class GUIManager;

		/**
		 * @brief	Initializes the event data with the character that was input.
		 */
		void setData(UINT32 inputChar);

		UINT32 mInputChar;
	};
}