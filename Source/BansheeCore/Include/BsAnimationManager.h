//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsModule.h"

namespace BansheeEngine
{
	struct AnimationProxy;

	/** @addtogroup Animation-Internal
	 *  @{
	 */
	
	/** 
	 * Keeps track of all active animations, queues animation thread tasks and synchronizes data between simulation, core
	 * and animation threads.
	 */
	class AnimationManager : public Module<AnimationManager>
	{
	public:
		AnimationManager();

		/** Pauses or resumes the animation evaluation. */
		void setPaused(bool paused);

		/** 
		 * Determines how often to evaluate animations. If rendering is not running at adequate framerate the animation
		 * could end up being evaluated less times than specified here.
		 *
		 * @param[in]	fps		Number of frames per second to evaluate the animation. Default is 60.
		 */
		void setUpdateRate(UINT32 fps);

		/** 
		 * Synchronizes animation data from the animation thread with the scene objects. Should be called before component
		 * updates are sent. 
		 */
		void preUpdate();

		/**
		 * Synchronizes animation data to the animation thread, advances animation time and queues new animation evaluation
		 * task.
		 */
		void postUpdate();

	private:
		friend class Animation;

		/** 
		 * Registers a new animation and returns a unique ID for it. Must be called whenever an Animation is constructed. 
		 */
		UINT64 registerAnimation(Animation* anim);

		/** Unregisters an animation with the specified ID. Must be called before an Animation is destroyed. */
		void unregisterAnimation(UINT64 id);

		/** Worker method ran on the animation thread that evaluates all animation at the provided time. */
		void evaluateAnimation();

		UINT64 mNextId;
		UnorderedMap<UINT64, Animation*> mAnimations;
		
		float mUpdateRate;
		float mAnimationTime;
		float mLastAnimationUpdateTime;
		float mNextAnimationUpdateTime;
		bool mPaused;

		bool mWorkerRunning;
		SPtr<Task> mAnimationWorker;

		Vector<SPtr<AnimationProxy>> mProxies;
	};

	/** @} */
}