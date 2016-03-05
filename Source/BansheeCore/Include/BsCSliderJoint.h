//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsSliderJoint.h"
#include "BsCJoint.h"

namespace BansheeEngine 
{
	/** @addtogroup Components-Core
	 *  @{
	 */

	/**
	 * @copydoc	SliderJoint
	 *
	 * Wraps SliderJoint as a Component.
	 */
    class BS_CORE_EXPORT CSliderJoint : public CJoint
    {
    public:
		CSliderJoint(const HSceneObject& parent);

		/** @copydoc SliderJoint::getPosition */
		inline float getPosition() const;

		/** @copydoc SliderJoint::getSpeed */
		inline float getSpeed() const;

		/** @copydoc SliderJoint::getLimit */
		inline LimitLinearRange getLimit() const;

		/** @copydoc SliderJoint::setLimit */
		inline void setLimit(const LimitLinearRange& limit);

		/** @copydoc SliderJoint::setFlag */
		inline void setFlag(SliderJoint::Flag flag, bool enabled);

		/** @copydoc SliderJoint::hasFlag */
		inline bool hasFlag(SliderJoint::Flag flag) const;

		/** @cond INTERNAL */

	    /**	Returns the slider joint that this component wraps. */
		SliderJoint* _getInternal() const { return static_cast<SliderJoint*>(mInternal.get()); }

		/** @endcond */

		/************************************************************************/
		/* 						COMPONENT OVERRIDES                      		*/
		/************************************************************************/
	protected:
		friend class SceneObject;

		/** @copydoc CJoint::createInternal */
		SPtr<Joint> createInternal() override;

		SliderJoint::Flag mFlag = (SliderJoint::Flag)0;
		LimitLinearRange mLimit;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class CSliderJointRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;

	protected:
		CSliderJoint() {} // Serialization only
     };

	 /** @} */
}