#pragma once

#include "BsPrerequisitesUtil.h"
#include "BsVector3.h"

namespace BansheeEngine
{
	/** @addtogroup Math
	 *  @{
	 */

	/** Represents a line segment in three dimensional space defined by a start and an end point. */
	class BS_UTILITY_EXPORT LineSegment3
	{
	public:
		LineSegment3();
		LineSegment3(const Vector3& start, const Vector3& end);

		/**
		 * Find the nearest point on the line segment and the provided ray.
		 *
		 * @return	Set of nearest points and distance from the points. First nearest point is a point along the ray, 
		 *			while the second is along the line segment.
		 *
		 * @note	If segment and ray are parallel the set of points at the segment origin are returned.
		 */
		std::pair<std::array<Vector3, 2>, float> getNearestPoint(const Ray& ray) const;

		/** Returns the starting point of the line segment. */
		const Vector3& getStart() const { return mStart; }

		/** Returns the ending point of the line segment. */
		const Vector3& getEnd() const { return mEnd; }
	private:
		Vector3 mStart, mEnd;
	};

	/** @} */
}