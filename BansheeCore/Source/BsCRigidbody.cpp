//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsCRigidbody.h"
#include "BsSceneObject.h"
#include "BsCCollider.h"
#include "BsCRigidbodyRTTI.h"

using namespace std::placeholders;

namespace BansheeEngine
{
	CRigidbody::CRigidbody(const HSceneObject& parent)
		: Component(parent)
	{
		setName("Rigidbody");
	}

	void CRigidbody::move(const Vector3& position)
	{
		if (mInternal != nullptr)
			mInternal->move(position);
	}

	void CRigidbody::rotate(const Quaternion& rotation)
	{
		if (mInternal != nullptr)
			mInternal->rotate(rotation);
	}

	void CRigidbody::setMass(float mass)
	{
		mMass = mass;

		if(mInternal != nullptr)
			mInternal->setMass(mass);
	}

	void CRigidbody::setIsKinematic(bool kinematic)
	{
		mIsKinematic = kinematic;
		
		if (mInternal != nullptr)
			mInternal->setIsKinematic(kinematic);
	}

	bool CRigidbody::isSleeping() const
	{
		if (mInternal != nullptr)
			return mInternal->isSleeping();

		return true;
	}

	void CRigidbody::sleep()
	{
		if (mInternal != nullptr)
			return mInternal->sleep();
	}

	void CRigidbody::wakeUp()
	{
		if (mInternal != nullptr)
			return mInternal->wakeUp();
	}

	void CRigidbody::setSleepThreshold(float threshold)
	{
		mSleepThreshold = threshold;

		if (mInternal != nullptr)
			mInternal->setSleepThreshold(threshold);
	}

	void CRigidbody::setUseGravity(bool gravity)
	{
		mUseGravity = gravity;

		if (mInternal != nullptr)
			mInternal->setUseGravity(gravity);
	}

	void CRigidbody::setVelocity(const Vector3& velocity)
	{
		if (mInternal != nullptr)
			mInternal->setVelocity(velocity);
	}

	Vector3 CRigidbody::getVelocity() const
	{
		if (mInternal != nullptr)
			return mInternal->getVelocity();

		return Vector3::ZERO;
	}

	void CRigidbody::setAngularVelocity(const Vector3& velocity)
	{
		if (mInternal != nullptr)
			mInternal->setAngularVelocity(velocity);
	}

	inline Vector3 CRigidbody::getAngularVelocity() const
	{
		if (mInternal != nullptr)
			return mInternal->getAngularVelocity();

		return Vector3::ZERO;
	}

	void CRigidbody::setDrag(float drag)
	{
		mLinearDrag = drag;

		if (mInternal != nullptr)
			mInternal->setDrag(drag);
	}

	void CRigidbody::setAngularDrag(float drag)
	{
		mAngularDrag = drag;

		if (mInternal != nullptr)
			mInternal->setAngularDrag(drag);
	}

	void CRigidbody::setInertiaTensor(const Vector3& tensor)
	{
		mIntertiaTensor = tensor;

		if (mInternal != nullptr)
			mInternal->setInertiaTensor(tensor);
	}

	Vector3 CRigidbody::getInertiaTensor() const
	{
		if (mInternal != nullptr)
			return mInternal->getInertiaTensor();

		return Vector3::ZERO;
	}

	void CRigidbody::setMaxAngularVelocity(float maxVelocity)
	{
		mMaxAngularVelocity = maxVelocity;

		if (mInternal != nullptr)
			mInternal->setMaxAngularVelocity(maxVelocity);
	}

	void CRigidbody::setCenterOfMass(const Vector3& position, const Quaternion& rotation)
	{
		mCMassPosition = position;
		mCMassRotation = rotation;

		if (mInternal != nullptr)
			mInternal->setCenterOfMass(position, rotation);
	}

	Vector3 CRigidbody::getCenterOfMassPosition() const
	{
		if (mInternal != nullptr)
			return mInternal->getCenterOfMassPosition();

		return Vector3::ZERO;
	}

	Quaternion CRigidbody::getCenterOfMassRotation() const
	{
		if (mInternal != nullptr)
			return mInternal->getCenterOfMassRotation();

		return Quaternion::IDENTITY;
	}

	void CRigidbody::setPositionSolverCount(UINT32 count)
	{
		mPositionSolverCount = count;

		if (mInternal != nullptr)
			mInternal->setPositionSolverCount(count);
	}

	void CRigidbody::setVelocitySolverCount(UINT32 count)
	{
		mVelocitySolverCount = count;

		if (mInternal != nullptr)
			mInternal->setVelocitySolverCount(count);
	}

	void CRigidbody::setFlags(Rigidbody::Flag flags)
	{
		mFlags = flags;

		if (mInternal != nullptr)
			mInternal->setFlags(flags);
	}

	void CRigidbody::addForce(const Vector3& force, ForceMode mode)
	{
		if (mInternal != nullptr)
			mInternal->addForce(force, mode);
	}

	void CRigidbody::addTorque(const Vector3& torque, ForceMode mode)
	{
		if (mInternal != nullptr)
			mInternal->addTorque(torque, mode);
	}

	void CRigidbody::addForceAtPoint(const Vector3& force, const Vector3& position, PointForceMode mode)
	{
		if (mInternal != nullptr)
			mInternal->addForceAtPoint(force, position, mode);
	}

	Vector3 CRigidbody::getVelocityAtPoint(const Vector3& point) const
	{
		if (mInternal != nullptr)
			return mInternal->getVelocityAtPoint(point);

		return Vector3::ZERO;
	}

	void CRigidbody::_updateMassDistribution()
	{
		if (mInternal != nullptr)
			return mInternal->_updateMassDistribution();
	}

	void CRigidbody::updateChildColliders()
	{
		Stack<HSceneObject> todo;
		todo.push(SO());

		while(!todo.empty())
		{
			HSceneObject currentSO = todo.top();
			todo.pop();

			if(currentSO->hasComponent<CCollider>())
			{
				Vector<HCollider> colliders = currentSO->getComponents<CCollider>();
				
				for (auto& entry : colliders)
					entry->_setRigidbody(mThisHandle);
			}

			UINT32 childCount = currentSO->getNumChildren();
			for (UINT32 i = 0; i < childCount; i++)
			{
				HSceneObject child = currentSO->getChild(i);

				if (child->hasComponent<CRigidbody>())
					continue;

				todo.push(child);
			}
		}
	}

	void CRigidbody::clearChildColliders()
	{
		if (mInternal == nullptr)
			return;

		mInternal->_detachColliders();
	}

	void CRigidbody::checkForNestedRigibody()
	{
		HSceneObject currentSO = SO()->getParent();

		while(currentSO != nullptr)
		{
			if(currentSO->hasComponent<CRigidbody>())
			{
				LOGWRN("Nested Rigidbodies detected. This will result in inconsistent transformations. To parent one " \
					"Rigidbody to another move its colliders to the new parent, but remove the Rigidbody component.");
				return;
			}

			currentSO = currentSO->getParent();
		}
	}

	void CRigidbody::triggerOnCollisionBegin(const CollisionData& data)
	{
		onCollisionBegin(data);
	}

	void CRigidbody::triggerOnCollisionStay(const CollisionData& data)
	{
		onCollisionStay(data);
	}

	void CRigidbody::triggerOnCollisionEnd(const CollisionData& data)
	{
		onCollisionEnd(data);
	}

	void CRigidbody::onInitialized()
	{

	}

	void CRigidbody::onDestroyed()
	{
		clearChildColliders();
		mInternal = nullptr;
	}

	void CRigidbody::onDisabled()
	{
		clearChildColliders();
		mInternal = nullptr;
	}

	void CRigidbody::onEnabled()
	{
		mInternal = Rigidbody::create(SO());

		updateChildColliders();

#if BS_DEBUG_MODE
		checkForNestedRigibody();
#endif

		mInternal->onCollisionBegin.connect(std::bind(&CRigidbody::triggerOnCollisionBegin, this, _1));
		mInternal->onCollisionStay.connect(std::bind(&CRigidbody::triggerOnCollisionStay, this, _1));
		mInternal->onCollisionEnd.connect(std::bind(&CRigidbody::triggerOnCollisionEnd, this, _1));

		mInternal->setTransform(SO()->getWorldPosition(), SO()->getWorldRotation());

		// Note: Merge into one call to avoid many virtual function calls
		mInternal->setPositionSolverCount(mPositionSolverCount);
		mInternal->setVelocitySolverCount(mVelocitySolverCount);
		mInternal->setMaxAngularVelocity(mMaxAngularVelocity);
		mInternal->setDrag(mLinearDrag);
		mInternal->setAngularDrag(mAngularDrag);
		mInternal->setSleepThreshold(mSleepThreshold);
		mInternal->setUseGravity(mUseGravity);
		mInternal->setIsKinematic(mIsKinematic);

		if(((UINT32)mFlags & (UINT32)Rigidbody::Flag::AutoTensors) == 0)
		{
			mInternal->setCenterOfMass(mCMassPosition, mCMassRotation);
			mInternal->setInertiaTensor(mIntertiaTensor);
			mInternal->setMass(mMass);
		}
		else
		{
			if (((UINT32)mFlags & (UINT32)Rigidbody::Flag::AutoMass) == 0)
				mInternal->setMass(mMass);

			mInternal->_updateMassDistribution();
		}
	}

	void CRigidbody::onTransformChanged(TransformChangedFlags flags)
	{
		if (!SO()->getActive())
			return;

		if((flags & TCF_Parent) != 0)
		{
			clearChildColliders();
			updateChildColliders();

			if (((UINT32)mFlags & (UINT32)Rigidbody::Flag::AutoTensors) != 0)
				mInternal->_updateMassDistribution();

#if BS_DEBUG_MODE
			checkForNestedRigibody();
#endif
		}

		mInternal->setTransform(SO()->getWorldPosition(), SO()->getWorldRotation());
	}

	RTTITypeBase* CRigidbody::getRTTIStatic()
	{
		return CRigidbodyRTTI::instance();
	}

	RTTITypeBase* CRigidbody::getRTTI() const
	{
		return CRigidbody::getRTTIStatic();
	}
}