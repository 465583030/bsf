//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"

namespace BansheeEngine
{
	/** @addtogroup Animation
	 *  @{
	 */

	struct AnimationInstanceData
	{
	public:
		AnimationInstanceData();

		float time;
	private:
		template <class T> friend class TAnimationCurve;

		mutable UINT32 cachedKey;
	};

	/** @} */
}