//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsVector3.h"
#include "BsQuaternion.h"

namespace BansheeEngine
{
	class BS_CORE_EXPORT FCollider
	{
	public:
		virtual ~FCollider();

		virtual Vector3 getPosition() const = 0;
		virtual Quaternion getRotation() const = 0;
		virtual void setTransform(const Vector3& pos, const Quaternion& rotation) = 0;

		virtual void setIsTrigger(bool value) = 0;
		virtual bool getIsTrigger() const = 0;

		virtual void setRigidbody(const SPtr<Rigidbody>& rigidbody);
		virtual SPtr<Rigidbody> getRigidbody() const { return mRigidbody; }

		virtual void setMaterial(const HPhysicsMaterial& material);
		virtual HPhysicsMaterial getMaterial() const { return mMaterial; }

		virtual void setIsActive(bool value);
		virtual bool getIsActive() const { return mIsActive; }

		virtual void setContactOffset(float value) = 0;
		virtual float getContactOffset() = 0;

		virtual void setRestOffset(float value) = 0;
		virtual float getRestOffset() = 0;

		virtual void setLayer(UINT64 layer);
		virtual UINT64 getLayer() const { return mLayer; }
	protected:
		bool mIsActive = true;
		UINT64 mLayer = 1;

		SPtr<Rigidbody> mRigidbody;
		HPhysicsMaterial mMaterial;
	};
}