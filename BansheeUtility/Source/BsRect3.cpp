#include "BsRect3.h"
#include "BsRay.h"
#include "BsLineSegment3.h"

namespace BansheeEngine
{
	Rect3::Rect3(const Vector3& center, const std::array<Vector3, 2>& axes,
		const std::array<float, 2>& extents)
		:mCenter(center), mAxes(axes), mExtents(extents)
	{

	}

	std::pair<std::array<Vector3, 2>, float> Rect3::getNearestPoint(const Ray& ray) const
	{
		const Vector3& org = ray.getOrigin();
		const Vector3& dir = ray.getDirection();

		bool foundNearest = false;
		float t = 0.0f;
		std::array<Vector3, 2> nearestPoints;
		float distance = 0.0f;

		// Check if Ray intersects the rectangle
		auto intersectResult = intersects(ray);
		if (intersectResult.first)
		{
			t = intersectResult.second;

			nearestPoints[0] = org + dir * t;
			nearestPoints[1] = nearestPoints[0]; // Just one point of intersection
			foundNearest = true;
		}

		// Ray is either passing next to the rectangle or parallel to it, 
		// compare ray to 4 edges of the rectangle
		if (!foundNearest)
		{
			Vector3 scaledAxes[2];
			scaledAxes[0] = mExtents[0] * mAxes[0];
			scaledAxes[1] = mExtents[1] * mAxes[1];

			distance = std::numeric_limits<float>::max();
			for (UINT32 i = 0; i < 2; i++)
			{
				for (UINT32 j = 0; j < 2; j++)
				{
					float sign = (float)(2 * j - 1);
					Vector3 segCenter = mCenter + sign * scaledAxes[i];
					Vector3 segStart = segCenter - scaledAxes[1 - i];
					Vector3 segEnd = segCenter + scaledAxes[1 - i];

					LineSegment3 segment(segStart, segEnd);
					auto segResult = segment.getNearestPoint(ray);

					if (segResult.second < distance)
					{
						nearestPoints = segResult.first;
						distance = segResult.second;
					}
				}
			}
		}

		// Front of the ray is nearest, use found points
		if (t >= 0.0f)
		{
			// Do nothing, we already have the points
		}
		else // Rectangle is behind the ray origin, find nearest point to origin
		{
			auto nearestPointToOrg = getNearestPoint(org);

			nearestPoints[0] = org;
			nearestPoints[1] = nearestPointToOrg.first;
			distance = nearestPointToOrg.second;
		}

		return std::make_pair(nearestPoints, distance);
	}

	std::pair<Vector3, float> Rect3::getNearestPoint(const Vector3& point) const
	{
		Vector3 diff = mCenter - point;
		float b0 = diff.dot(mAxes[0]);
		float b1 = diff.dot(mAxes[1]);
		float s0 = -b0, s1 = -b1;
		float sqrDistance = diff.dot(diff);

		if (s0 < -mExtents[0])
			s0 = -mExtents[0];
		else if (s0 > mExtents[0])
			s0 = mExtents[0];

		sqrDistance += s0*(s0 + 2.0f*b0);

		if (s1 < -mExtents[1])
			s1 = -mExtents[1];
		else if (s1 > mExtents[1])
			s1 = mExtents[1];

		sqrDistance += s1*(s1 + 2.0f*b1);

		if (sqrDistance < 0.0f)
			sqrDistance = 0.0f;

		float dist = std::sqrt(sqrDistance);
		Vector3 nearestPoint = mCenter + s0 * mAxes[0] + s1 * mAxes[1];

		return std::make_pair(nearestPoint, dist);
	}

	std::pair<bool, float> Rect3::intersects(const Ray& ray) const
	{
		const Vector3& org = ray.getOrigin();
		const Vector3& dir = ray.getDirection();

		Vector3 normal = mAxes[0].cross(mAxes[1]);
		float NdotD = normal.dot(ray.getDirection());
		if (fabs(NdotD) > 0.0f)
		{
			Vector3 diff = ray.getOrigin() - mCenter;
			Vector3 basis[3];

			basis[0] = ray.getDirection();
			basis[0].orthogonalComplement(basis[1], basis[2]);

			float UdD0 = basis[1].dot(mAxes[0]);
			float UdD1 = basis[1].dot(mAxes[1]);
			float UdPmC = basis[1].dot(diff);
			float VdD0 = basis[2].dot(mAxes[0]);
			float VdD1 = basis[2].dot(mAxes[1]);
			float VdPmC = basis[2].dot(diff);
			float invDet = 1.0f / (UdD0*VdD1 - UdD1*VdD0);

			float s0 = (VdD1*UdPmC - UdD1*VdPmC)*invDet;
			float s1 = (UdD0*VdPmC - VdD0*UdPmC)*invDet;

			if (fabs(s0) <= mExtents[0] && fabs(s1) <= mExtents[1])
			{
				float DdD0 = dir.dot(mAxes[0]);
				float DdD1 = dir.dot(mAxes[1]);
				float DdDiff = dir.dot(diff);

				float t = s0 * DdD0 + s1 * DdD1 - DdDiff;

				return std::make_pair(true, t);
			}
		}

		return std::make_pair(false, 0.0f);
	}
}