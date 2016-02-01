//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPhysXPrerequisites.h"
#include "BsPlaneCollider.h"
#include "PxPhysics.h"

namespace BansheeEngine
{
	class PhysXPlaneCollider : public PlaneCollider
	{
	public:
		PhysXPlaneCollider(physx::PxPhysics* physx, const Vector3& position, const Quaternion& rotation);
		~PhysXPlaneCollider();

	private:
		FPhysXCollider* getInternal() const;
	};
}