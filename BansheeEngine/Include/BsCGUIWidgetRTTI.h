//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsRTTIType.h"
#include "BsCGUIWidget.h"
#include "BsGameObjectRTTI.h"

namespace BansheeEngine
{
	/** @cond INTERNAL */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class BS_EXPORT CGUIWidgetRTTI : public RTTIType <CGUIWidget, Component, CGUIWidgetRTTI>
	{
	private:

	public:
		CGUIWidgetRTTI()
		{ }

		const String& getRTTIName() override
		{
			static String name = "CGUIWidget";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_CGUIWidget;
		}

		std::shared_ptr<IReflectable> newRTTIObject() override
		{
			return GameObjectRTTI::createGameObject<CGUIWidget>();
		}
	};

	/** @} */
	/** @endcond */
}