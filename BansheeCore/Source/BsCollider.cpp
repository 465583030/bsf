#include "BsCollider.h"
#include "BsFCollider.h"

namespace BansheeEngine
{
	Vector3 Collider::getPosition() const
	{
		return mInternal->getPosition();
	}

	Quaternion Collider::getRotation() const
	{
		return mInternal->getRotation();
	}

	void Collider::setTransform(const Vector3& pos, const Quaternion& rot)
	{
		mInternal->setTransform(pos, rot);
	}

	void Collider::setScale(const Vector3& scale)
	{
		mScale = scale;
	}

	Vector3 Collider::getScale() const
	{
		return mScale;
	}

	void Collider::setIsTrigger(bool value)
	{
		mInternal->setIsTrigger(value);
	}

	bool Collider::getIsTrigger() const
	{
		return mInternal->getIsTrigger();
	}

	void Collider::setRigidbody(const SPtr<Rigidbody>& value)
	{
		mInternal->setIsStatic(value == nullptr);

		mRigidbody = value;
	}

	void Collider::setMass(float mass)
	{
		mInternal->setMass(mass);
	}

	float Collider::getMass() const
	{
		return mInternal->getMass();
	}

	void Collider::setMaterial(const HPhysicsMaterial& material)
	{
		mInternal->setMaterial(material);
	}

	HPhysicsMaterial Collider::getMaterial() const
	{
		return mInternal->getMaterial();
	}

	void Collider::setContactOffset(float value)
	{
		mInternal->setContactOffset(value);
	}

	float Collider::getContactOffset()
	{
		return mInternal->getContactOffset();
	}

	void Collider::setRestOffset(float value)
	{
		mInternal->setRestOffset(value);
	}

	float Collider::getRestOffset()
	{
		return mInternal->getRestOffset();
	}

	void Collider::setLayer(UINT64 layer)
	{
		mInternal->setLayer(layer);
	}

	UINT64 Collider::getLayer() const
	{
		return mInternal->getLayer();
	}
}