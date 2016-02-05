//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsRigidbody.h"
#include "BsComponent.h"
#include "BsVector3.h"
#include "BsQuaternion.h"

namespace BansheeEngine
{
	/** @addtogroup Components-Core
	 *  @{
	 */

	/**
	 * @copydoc	Rigidbody
	 *
	 * Wraps Rigidbody as a Component.
	 */
	class BS_CORE_EXPORT CRigidbody : public Component
	{
	public:
		CRigidbody(const HSceneObject& parent);

		/** @copydoc Rigidbody::move */
		inline void move(const Vector3& position);

		/** @copydoc Rigidbody::rotate */
		inline void rotate(const Quaternion& rotation);

		/** @copydoc Rigidbody::setMass */
		inline void setMass(float mass);

		/** @copydoc Rigidbody::getMass */
		float getMass() const { return mMass; };

		/** @copydoc Rigidbody::setIsKinematic */
		inline void setIsKinematic(bool kinematic);

		/** @copydoc Rigidbody::getIsKinematic */
		bool getIsKinematic() const { return mIsKinematic; }

		/** @copydoc Rigidbody::isSleeping */
		inline bool isSleeping() const;

		/** @copydoc Rigidbody::sleep */
		inline void sleep();

		/** @copydoc Rigidbody::wakeUp */
		inline void wakeUp();

		/** @copydoc Rigidbody::setSleepThreshold */
		inline void setSleepThreshold(float threshold);

		/** @copydoc Rigidbody::getSleepThreshold */
		float getSleepThreshold() const { return mSleepThreshold; }

		/** @copydoc Rigidbody::setUseGravity */
		inline void setUseGravity(bool gravity);

		/** @copydoc Rigidbody::getUseGravity */
		bool getUseGravity() const { return mUseGravity; }

		/** @copydoc Rigidbody::setVelocity */
		inline void setVelocity(const Vector3& velocity);

		/** @copydoc Rigidbody::getVelocity */
		inline Vector3 getVelocity() const;

		/** @copydoc Rigidbody::setAngularVelocity */
		inline void setAngularVelocity(const Vector3& velocity);

		/** @copydoc Rigidbody::getAngularVelocity */
		inline Vector3 getAngularVelocity() const;

		/** @copydoc Rigidbody::setDrag */
		inline void setDrag(float drag);

		/** @copydoc Rigidbody::getDrag */
		float getDrag() const { return mLinearDrag; }

		/** @copydoc Rigidbody::setAngularDrag */
		inline void setAngularDrag(float drag);

		/** @copydoc Rigidbody::getAngularDrag */
		float getAngularDrag() const { return mAngularDrag; }

		/** @copydoc Rigidbody::setInertiaTensor */
		inline void setInertiaTensor(const Vector3& tensor);

		/** @copydoc Rigidbody::getInertiaTensor */
		inline Vector3 getInertiaTensor() const;

		/** @copydoc Rigidbody::setMaxAngularVelocity */
		inline void setMaxAngularVelocity(float maxVelocity);

		/** @copydoc Rigidbody::getMaxAngularVelocity */
		float getMaxAngularVelocity() const { return mMaxAngularVelocity; }

		/** @copydoc Rigidbody::setCenterOfMass */
		inline void setCenterOfMass(const Vector3& position, const Quaternion& rotation);

		/** @copydoc Rigidbody::getCenterOfMassPosition */
		inline  Vector3 getCenterOfMassPosition() const;

		/** @copydoc Rigidbody::getCenterOfMassRotation */
		inline Quaternion getCenterOfMassRotation() const;

		/** @copydoc Rigidbody::setPositionSolverCount */
		inline void setPositionSolverCount(UINT32 count);

		/** @copydoc Rigidbody::getPositionSolverCount */
		UINT32 getPositionSolverCount() const { return mPositionSolverCount; }

		/** @copydoc Rigidbody::setVelocitySolverCount */
		inline void setVelocitySolverCount(UINT32 count);

		/** @copydoc Rigidbody::getVelocitySolverCount */
		UINT32 getVelocitySolverCount() const { return mVelocitySolverCount; }

		/** @copydoc Rigidbody::setFlags */
		inline void setFlags(Rigidbody::Flag flags);

		/** @copydoc Rigidbody::getFlags */
		Rigidbody::Flag getFlags() const { return mFlags; }

		/** @copydoc Rigidbody::addForce */
		inline void addForce(const Vector3& force, ForceMode mode = ForceMode::Force);

		/** @copydoc Rigidbody::addTorque */
		inline void addTorque(const Vector3& torque, ForceMode mode = ForceMode::Force);

		/** @copydoc Rigidbody::addForceAtPoint */
		inline void addForceAtPoint(const Vector3& force, const Vector3& position,
			PointForceMode mode = PointForceMode::Force);

		/** @copydoc Rigidbody::getVelocityAtPoint */
		inline Vector3 getVelocityAtPoint(const Vector3& point) const;

		/** @copydoc Rigidbody::onCollisionBegin */
		Event<void(const CollisionData&)> onCollisionBegin;

		/** @copydoc Rigidbody::onCollisionStay */
		Event<void(const CollisionData&)> onCollisionStay;

		/** @copydoc Rigidbody::onCollisionEnd */
		Event<void(const CollisionData&)> onCollisionEnd;

		/** @cond INTERNAL */

		/** Returns the Rigidbody implementation wrapped by this component. */
		SPtr<Rigidbody> _getInternal() const { return mInternal; }

		/** @copydoc Rigidbody::_updateMassDistribution */
		inline void _updateMassDistribution();

		/** @endcond */
	protected:
		/** 
		 * Searches child scene objects for Collider components and attaches them to the rigidbody. Make sure to call
		 * clearChildColliders() if you need to clear old colliders first. 
		 */
		void updateChildColliders();

		/** Unregisters all child colliders from the Rigidbody. */
		void clearChildColliders();

		/** Checks if the rigidbody is nested under another rigidbody, and throws out a warning if so. */
		void checkForNestedRigibody();

		/** Triggered when the internal rigidbody begins touching another object. */
		void triggerOnCollisionBegin(const CollisionData& data);

		/** Triggered when the internal rigidbody continues touching another object. */
		void triggerOnCollisionStay(const CollisionData& data);

		/** Triggered when the internal rigidbody ends touching another object. */
		void triggerOnCollisionEnd(const CollisionData& data);

		/************************************************************************/
		/* 						COMPONENT OVERRIDES                      		*/
		/************************************************************************/
	protected:
		friend class SceneObject;

		/** @copydoc Component::onInitialized() */
		void onInitialized() override;

		/** @copydoc Component::onDestroyed() */
		void onDestroyed() override;

		/** @copydoc Component::onDisabled() */
		void onDisabled() override;

		/** @copydoc Component::onEnabled() */
		void onEnabled() override;

		/** @copydoc Component::onTransformChanged() */
		void onTransformChanged(TransformChangedFlags flags) override;

		SPtr<Rigidbody> mInternal;

		UINT32 mPositionSolverCount = 4;
		UINT32 mVelocitySolverCount = 1;
		Rigidbody::Flag mFlags = (Rigidbody::Flag)((UINT32)Rigidbody::Flag::AutoTensors | (UINT32)Rigidbody::Flag::AutoMass);
		Vector3 mCMassPosition;
		Quaternion mCMassRotation;
		Vector3 mIntertiaTensor;
		float mMass = 0.0f;
		float mMaxAngularVelocity = 1.0f;
		float mLinearDrag = 0.0f;
		float mAngularDrag = 0.0f;
		float mSleepThreshold = 0.0f;
		bool mUseGravity = true;
		bool mIsKinematic = false;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class CRigidbodyRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;

	protected:
		CRigidbody() {} // Serialization only
	};

	/** @} */
}