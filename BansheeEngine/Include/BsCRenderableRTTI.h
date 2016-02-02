//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "BsRTTIType.h"
#include "BsCRenderable.h"
#include "BsGameObjectRTTI.h"

namespace BansheeEngine
{
	/** @cond INTERNAL */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class BS_EXPORT CRenderableRTTI : public RTTIType<CRenderable, Component, CRenderableRTTI>
	{
	private:
		RenderablePtr getInternal(CRenderable* obj) { return obj->mInternal; }
		void setInternal(CRenderable* obj, RenderablePtr val) { obj->mInternal = val; }

	public:
		CRenderableRTTI()
		{
			addReflectablePtrField("mInternal", 0, &CRenderableRTTI::getInternal, &CRenderableRTTI::setInternal);
		}

		const String& getRTTIName() override
		{
			static String name = "CRenderable";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_CRenderable;
		}

		std::shared_ptr<IReflectable> newRTTIObject() override
		{
			return GameObjectRTTI::createGameObject<CRenderable>();
		}
	};

	/** @} */
	/** @endcond */
}