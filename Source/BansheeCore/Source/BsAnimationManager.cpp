//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsAnimationManager.h"
#include "BsAnimation.h"
#include "BsAnimationClip.h"
#include "BsTaskScheduler.h"
#include "BsTime.h"
#include "BsCoreSceneManager.h"
#include "BsCamera.h"

namespace BansheeEngine
{
	AnimationManager::AnimationManager()
		: mNextId(1), mUpdateRate(1.0f / 60.0f), mAnimationTime(0.0f), mLastAnimationUpdateTime(0.0f)
		, mNextAnimationUpdateTime(0.0f), mPaused(false), mWorkerRunning(false), mPoseReadBufferIdx(1)
		, mPoseWriteBufferIdx(0), mDataReadyCount(0), mDataReady(false)
	{
		mAnimationWorker = Task::create("Animation", std::bind(&AnimationManager::evaluateAnimation, this));
	}

	void AnimationManager::setPaused(bool paused)
	{
		mPaused = paused;
	}

	void AnimationManager::setUpdateRate(UINT32 fps)
	{
		if (fps == 0)
			fps = 1;

		mUpdateRate = 1.0f / fps;
	}

	void AnimationManager::preUpdate()
	{
		if (mPaused || !mWorkerRunning)
			return;

		mAnimationWorker->wait();

		// Make sure we don't load obsolete skeletal pose and other evaluation ouputs written by the animation thread
		std::atomic_thread_fence(std::memory_order_acquire);

		// Trigger events
		for (auto& anim : mAnimations)
		{
			anim.second->updateFromProxy();
			anim.second->triggerEvents(mAnimationTime, gTime().getFrameDelta());
		}

		mWorkerRunning = false;
	}

	void AnimationManager::postUpdate()
	{
		if (mPaused)
			return;

		mAnimationTime += gTime().getFrameDelta();
		if (mAnimationTime < mNextAnimationUpdateTime)
			return;

		mNextAnimationUpdateTime = Math::floor(mAnimationTime / mUpdateRate) * mUpdateRate + mUpdateRate;

		float timeDelta = mAnimationTime - mLastAnimationUpdateTime;
		mLastAnimationUpdateTime = mAnimationTime;

		// Update poses in the currently active buffer. Multi-buffering allows the core thread to safely read the
		// poses without worrying about them being overwritten by another call to postUpdate, as long as the simulation
		// thread doesn't go more than (CoreThread::NUM_SYNC_BUFFERS - 1) frames ahead.
		mProxies.clear();
		for (auto& anim : mAnimations)
		{
			anim.second->updateAnimProxy(timeDelta);
			mProxies.push_back(anim.second->mAnimProxy);
		}

		mCullFrustums.clear();

		auto& allCameras = gCoreSceneManager().getAllCameras();
		for(auto& entry : allCameras)
		{
			bool isOverlayCamera = entry.second.camera->getFlags().isSet(CameraFlag::Overlay);
			if (isOverlayCamera)
				continue;

			// TODO: Not checking if camera and animation renderable's layers match. If we checked more animations could
			// be culled.
			mCullFrustums.push_back(entry.second.camera->getWorldFrustum());
		}

		// Make sure thread finishes writing all changes to the anim proxies as they will be read by the animation thread
		std::atomic_thread_fence(std::memory_order_release);
		
		// Note: Animation thread will trigger about the same time as the core thread. The core thread will need to wait
		// until animation thread finishes, which might end up blocking it (and losing the multi-threading performance). 
		// Consider delaying displayed animation for a single frame or pre-calculating animations (by advancing time the
		// previous frame) for non-dirty animations.
		TaskScheduler::instance().addTask(mAnimationWorker);
		mWorkerRunning = true;
	}

	void AnimationManager::evaluateAnimation()
	{
		// Make sure we don't load obsolete anim proxy data written by the simulation thread
		std::atomic_thread_fence(std::memory_order_acquire);

		// No need for locking, as we are sure that only postUpdate() writes to the proxy buffer, and increments the write
		// buffer index. And it's called sequentially ensuring previous call to evaluate finishes.

		UINT32 totalNumBones = 0;
		for (auto& anim : mProxies)
		{
			if (anim->skeleton != nullptr)
				totalNumBones += anim->skeleton->getNumBones();
		}

		RendererAnimationData& renderData = mAnimData[mPoseWriteBufferIdx];
		mPoseWriteBufferIdx = (mPoseWriteBufferIdx + 1) % CoreThread::NUM_SYNC_BUFFERS;

		renderData.poseInfos.clear();
		renderData.transforms.resize(totalNumBones);

		UINT32 curBoneIdx = 0;
		for(auto& anim : mProxies)
		{
			if(anim->mCullEnabled)
			{
				bool isVisible = false;
				for(auto& frustum : mCullFrustums)
				{
					if(frustum.intersects(anim->mBounds))
					{
						isVisible = true;
						break;
					}
				}

				if (!isVisible)
					continue;
			}

			if (anim->skeleton != nullptr)
			{
				UINT32 numBones = anim->skeleton->getNumBones();

				RendererAnimationData::PoseInfo info;
				info.animId = anim->id;
				info.startIdx = curBoneIdx;
				info.numBones = numBones;

				memset(anim->skeletonPose.hasOverride, 0, sizeof(bool) * anim->skeletonPose.numBones);
				Matrix4* boneDst = renderData.transforms.data() + curBoneIdx;

				// Copy transforms from mapped scene objects
				UINT32 boneTfrmIdx = 0;
				for(UINT32 i = 0; i < anim->numSceneObjects; i++)
				{
					const AnimatedSceneObjectInfo& soInfo = anim->sceneObjectInfos[i];

					if (soInfo.boneIdx == -1)
						continue;

					boneDst[soInfo.boneIdx] = anim->sceneObjectTransforms[boneTfrmIdx];
					anim->skeletonPose.hasOverride[soInfo.boneIdx] = true;
					boneTfrmIdx++;
				}

				// Animate bones
				anim->skeleton->getPose(boneDst, anim->skeletonPose, anim->skeletonMask, anim->layers, anim->numLayers);

				renderData.poseInfos[anim->id] = info;
				curBoneIdx += numBones;
			}

			// Reset mapped SO transform
			for (UINT32 i = 0; i < anim->sceneObjectPose.numBones; i++)
			{
				anim->sceneObjectPose.positions[i] = Vector3::ZERO;
				anim->sceneObjectPose.rotations[i] = Quaternion::IDENTITY;
				anim->sceneObjectPose.scales[i] = Vector3::ONE;
			}

			// Update mapped scene objects
			for(UINT32 i = 0; i < anim->numSceneObjects; i++)
			{
				const AnimatedSceneObjectInfo& soInfo = anim->sceneObjectInfos[i];

				// We already evaluated bones
				if (soInfo.boneIdx != -1)
					continue;

				if (soInfo.layerIdx == (UINT32)-1 || soInfo.stateIdx == (UINT32)-1)
					continue;

				const AnimationState& state = anim->layers[soInfo.layerIdx].states[soInfo.stateIdx];
				if (state.disabled)
					continue;

				{
					UINT32 curveIdx = soInfo.curveIndices.position;
					if (curveIdx != (UINT32)-1)
					{
						const TAnimationCurve<Vector3>& curve = state.curves->position[curveIdx].curve;
						anim->sceneObjectPose.positions[curveIdx] = curve.evaluate(state.time, state.positionCaches[curveIdx], state.loop);
					}
				}

				{
					UINT32 curveIdx = soInfo.curveIndices.rotation;
					if (curveIdx != (UINT32)-1)
					{
						const TAnimationCurve<Quaternion>& curve = state.curves->rotation[curveIdx].curve;
						anim->sceneObjectPose.rotations[curveIdx] = curve.evaluate(state.time, state.rotationCaches[curveIdx], state.loop);
						anim->sceneObjectPose.rotations[curveIdx].normalize();
					}
				}

				{
					UINT32 curveIdx = soInfo.curveIndices.scale;
					if (curveIdx != (UINT32)-1)
					{
						const TAnimationCurve<Vector3>& curve = state.curves->scale[curveIdx].curve;
						anim->sceneObjectPose.scales[curveIdx] = curve.evaluate(state.time, state.scaleCaches[curveIdx], state.loop);
					}
				}
			}

			// Note: No blending for generic animations, just use first animation
			if (anim->numLayers > 0 && anim->layers[0].numStates > 0)
			{
				const AnimationState& state = anim->layers[0].states[0];
				if (state.disabled)
					continue;

				{
					UINT32 numCurves = (UINT32)state.curves->generic.size();
					for (UINT32 i = 0; i < numCurves; i++)
					{
						const TAnimationCurve<float>& curve = state.curves->generic[i].curve;
						anim->genericCurveOutputs[i] = curve.evaluate(state.time, state.genericCaches[i], state.loop);
					}
				}
			}
		}

		mDataReadyCount.fetch_add(1, std::memory_order_relaxed);

		// Make sure the thread finishes writing skeletal pose and other evaluation outputs as they will be read by sim and
		// core threads
		std::atomic_thread_fence(std::memory_order_release);
	}

	void AnimationManager::waitUntilComplete()
	{
		mAnimationWorker->wait();

		// Make sure we don't load obsolete skeletal pose and other evaluation ouputs written by the animation thread
		std::atomic_thread_fence(std::memory_order_acquire);

		INT32 dataReadyCount = mDataReadyCount.load(std::memory_order_relaxed);
		assert(dataReadyCount <= CoreThread::NUM_SYNC_BUFFERS);

		mDataReady = dataReadyCount > 0;
		if (!mDataReady)
			return;

		mDataReadyCount.fetch_add(-1, std::memory_order_relaxed);
		mPoseReadBufferIdx = (mPoseReadBufferIdx + 1) % CoreThread::NUM_SYNC_BUFFERS;
	}

	const RendererAnimationData& AnimationManager::getRendererData()
	{
		if (!mDataReady)
		{
			static RendererAnimationData dummy;
			return dummy;
		}

		return mAnimData[mPoseReadBufferIdx];
	}

	UINT64 AnimationManager::registerAnimation(Animation* anim)
	{
		mAnimations[mNextId] = anim;
		return mNextId++;
	}

	void AnimationManager::unregisterAnimation(UINT64 animId)
	{
		mAnimations.erase(animId);
	}

	AnimationManager& gAnimation()
	{
		return AnimationManager::instance();
	}
}