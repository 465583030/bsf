//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsIReflectable.h"
#include "BsMatrix4.h"
#include "BsVector3.h"
#include "BsQuaternion.h"
#include "BsCurveEvaluator.h"

namespace BansheeEngine
{
	/** @addtogroup Animation-Internal
	 *  @{
	 */

	/** Contains indices for position/rotation/scale animation curves. */
	struct AnimationCurveMapping
	{
		UINT32 position;
		UINT32 rotation;
		UINT32 scale;
	};

	struct BONE_DESC
	{
		String name;
		UINT32 parent;

		Matrix4 invBindPose;
	};

	struct AnimationState
	{
		SPtr<AnimationCurves> curves;
		Vector<AnimationCurveMapping> boneToCurveMapping;

		TCurveEvaluatorData<Vector3> positionEval;
		TCurveEvaluatorData<Quaternion> rotationEval;
		TCurveEvaluatorData<Vector3> scaleEval;

		float weight;
		bool loop;
	};

	struct AnimationStateLayer
	{
		AnimationState* states;
		UINT32 numStates;

		UINT8 index;
		bool normalizeWeights;
	};

	struct SkeletonPose
	{
		SkeletonPose(UINT32 numBones);
		~SkeletonPose();

		Matrix4* bonePoses; // Global bone poses
		Vector3* positions; // Local positions
		Quaternion* rotations; // Local rotations
		Vector3* scales; // Local scales
		UINT32 numBones;
	};

	struct SkeletonBoneInfo
	{
		String name;
		UINT32 parent;
	};

	class BS_CORE_EXPORT Skeleton : public IReflectable // Note: Must be immutable in order to be usable on multiple threads
	{
	public:
		~Skeleton();

		void getPose(SkeletonPose& pose, const AnimationClip& clip, float time, bool loop = true);
		void getPose(SkeletonPose& pose, const AnimationStateLayer* layers, UINT32 numLayers);

		UINT32 getNumBones() const { return mNumBones; }
		const SkeletonBoneInfo& getBoneInfo(UINT32 idx) const { return mBoneInfo[idx]; }
		const Matrix4& getBindPose(UINT32 idx) const { return mInvBindPoses[idx]; }

		static SPtr<Skeleton> create(BONE_DESC* bones, UINT32 numBones);

	private:
		Skeleton();
		Skeleton(BONE_DESC* bones, UINT32 numBones);

		UINT32 mNumBones;
		Matrix4* mInvBindPoses;
		SkeletonBoneInfo* mBoneInfo;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class SkeletonRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;

		/** 
		 * Creates a Skeleton with no data. You must populate its data manually.
		 *
		 * @note	For serialization use only.
		 */
		static SPtr<Skeleton> createEmpty();
	};

	/** @} */
}