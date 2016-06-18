//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsIReflectable.h"
#include "BsMatrix4.h"

namespace BansheeEngine
{
	/** @addtogroup Animation-Internal
	 *  @{
	 */

	struct BONE_DESC
	{
		String name;
		UINT32 parent;

		Matrix4 invBindPose;
	};

	struct ANIM_BLEND_STATE_DESC
	{
		const AnimationClip* clip;
		float weight;
		bool loop;
		UINT8 layer;
	};

	struct SkeletonPose
	{
		SkeletonPose(UINT32 numBones)
			:numBones(numBones), bonePoses(bs_newN<Matrix4>(numBones))
		{}

		~SkeletonPose()
		{
			bs_deleteN(bonePoses, numBones);
		}

		Matrix4* bonePoses;
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
		void getPose(SkeletonPose& pose, const ANIM_BLEND_STATE_DESC* states, UINT32 numStates, float time);

		UINT32 getNumBones() const { return mNumBones; }
		const SkeletonBoneInfo& getBoneInfo(UINT32 idx) const { return mBoneInfo[idx]; }
		const Matrix4& getBindPose(UINT32 idx) const { return mBindPoses[idx]; }

		static SPtr<Skeleton> create(BONE_DESC* bones, UINT32 numBones);

	private:
		Skeleton();
		Skeleton(BONE_DESC* bones, UINT32 numBones);

		UINT32 mNumBones;
		Matrix4* mBindPoses;
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