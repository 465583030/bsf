//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsPlaneCollider.h"
#include "BsCCollider.h"

namespace BansheeEngine 
{
	/** @addtogroup Components-Core
	 *  @{
	 */

	/**
	 * @copydoc	PlaneCollider
	 *
	 * Wraps PlaneCollider as a Component.
	 */
    class BS_CORE_EXPORT CPlaneCollider : public CCollider
    {
    public:
		CPlaneCollider(const HSceneObject& parent);

		/** Sets the normal vector of the plane. It determines how is the plane oriented. */
		void setNormal(const Vector3& normal);

		/** Gets the normal vector of the plane. It determines how is the plane oriented. */
		Vector3 getNormal() const { return mNormal; }

		/** Sets the distance of the plane from the local origin, along its normal vector. */
		void setDistance(float distance);

		/** Gets the distance of the plane from the local origin, along its normal vector. */
		float getDistance() const { return mDistance; }

		/** @cond INTERNAL */

	    /**	Returns the plane collider that this component wraps. */
		SPtr<PlaneCollider> _getInternal() const { return std::static_pointer_cast<PlaneCollider>(mInternal); }

		/** @endcond */

		/************************************************************************/
		/* 						COMPONENT OVERRIDES                      		*/
		/************************************************************************/
	protected:
		friend class SceneObject;

		/** @copydoc CCollider::createInternal */
		SPtr<Collider> createInternal() override;

		/** @copydoc CPlaneCollider::isValidParent */
		bool isValidParent(const HRigidbody& parent) const override;

    protected:
		Vector3 mNormal = Vector3::UNIT_Y;
		float mDistance = 0.0f;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class CPlaneColliderRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;

	protected:
		CPlaneCollider() {} // Serialization only
     };

	 /** @} */
}