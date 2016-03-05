//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsSphericalJoint.h"
#include "BsPhysics.h"

namespace BansheeEngine
{
	SPtr<SphericalJoint> SphericalJoint::create()
	{
		return gPhysics().createSphericalJoint();
	}
}