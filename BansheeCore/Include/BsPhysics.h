//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsModule.h"
#include "BsVector3.h"
#include "BsQuaternion.h"

namespace BansheeEngine
{
	class BS_CORE_EXPORT Physics : public Module<Physics>
	{
	public:
		Physics();
		virtual ~Physics() { }

		virtual void update() = 0;

		virtual SPtr<PhysicsMaterial> createMaterial(float staticFriction, float dynamicFriction, float restitution) = 0;
		virtual SPtr<Rigidbody> createRigidbody(const Vector3& position = Vector3::ZERO, 
			const Quaternion& rotation = Quaternion::IDENTITY) = 0;

		void toggleCollision(UINT64 groupA, UINT64 groupB, bool enabled);
		bool isCollisionEnabled(UINT64 groupA, UINT64 groupB) const;

		static const UINT64 CollisionMapSize = 64;
	protected:
		mutable Mutex mMutex;
		bool mCollisionMap[CollisionMapSize][CollisionMapSize];
	};

	/** Provides easier access to Physics. */
	BS_CORE_EXPORT Physics& gPhysics();
}