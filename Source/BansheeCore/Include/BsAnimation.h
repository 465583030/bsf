//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsCoreObject.h"
#include "BsFlags.h"
#include "BsSkeleton.h"
#include "BsVector2.h"

namespace BansheeEngine
{
	/** @addtogroup Animation-Internal
	 *  @{
	 */

	/** Determines how an animation clip behaves when it reaches the end. */
	enum class AnimWrapMode
	{
		Loop, /**< Loop around to the beginning/end when the last/first frame is reached. */
		Clamp /**< Clamp to end/beginning, keeping the last/first frame active. */
	};

	/** Flags that determine which portion of Animation was changed and needs to be updated. */
	enum class AnimDirtyStateFlag
	{
		Clean = 0,
		Value = 1 << 0,
		Layout = 1 << 1,
		Skeleton = 1 << 2
	};

	typedef Flags<AnimDirtyStateFlag> AnimDirtyState;
	BS_FLAGS_OPERATORS(AnimDirtyStateFlag)

	/** Contains information about a currently playing animation clip. */
	struct AnimationClipState
	{
		AnimationClipState() { }

		/** Layer the clip is playing on. Multiple clips can be played simulatenously on different layers. */
		UINT32 layer = 0; 
		float time = 0.0f; /**< Current time the animation is playing from. */
		float speed = 1.0f; /**< Speed at which the animation is playing. */
		float weight = 1.0f; /**< Determines how much of an influence does the clip have on the final pose. */
		/** Determines what happens to other animation clips when a new clip starts playing. */
		AnimWrapMode wrapMode = AnimWrapMode::Loop;
	};

	/** Internal information about a single playing animation clip within Animation. */
	struct AnimationClipInfo
	{
		AnimationClipInfo();
		AnimationClipInfo(const HAnimationClip& clip);

		HAnimationClip clip;
		AnimationClipState state;
		
		float fadeDirection;
		float fadeTime;
		float fadeLength;

		/** 
		 * Version of the animation curves used by the AnimationProxy. Used to detecting the internal animation curves
		 * changed. 
		 */
		UINT64 curveVersion; 
		UINT32 layerIdx; /**< Layer index this clip belongs to in AnimationProxy structure. */
		UINT32 stateIdx; /**< State index this clip belongs to in AnimationProxy structure. */
	};

	/** Defines a single animation clip in BlendSequentialInfo. */
	struct BS_CORE_EXPORT BlendSequentialClipInfo
	{
		BlendSequentialClipInfo() { }

		HAnimationClip clip;
		float fadeTime = 0.0f;
		float startTime = 0.0f;
		float endTime = 0.0f;
	};

	/** Defines a sequential blend where one animation clip is played after another, with an optional fade between them. */
	struct BS_CORE_EXPORT BlendSequentialInfo
	{
		BlendSequentialInfo(UINT32 numClips);
		~BlendSequentialInfo();

		BlendSequentialClipInfo* clips;
		UINT32 numClips;
	};

	/** Defines a 1D blend where two animation clips are blended between each other using linear interpolation. */
	struct Blend1DInfo
	{
		HAnimationClip leftClip;
		HAnimationClip rightClip;
	};

	/** Defines a 2D blend where two animation clips are blended between each other using bilinear interpolation. */
	struct Blend2DInfo
	{
		HAnimationClip topLeftClip;
		HAnimationClip topRightClip;
		HAnimationClip botLeftClip;
		HAnimationClip botRightClip;
	};

	/** Represents a copy of the Animation data for use specifically on the animation thread. */
	struct AnimationProxy
	{
		AnimationProxy(UINT64 id);
		~AnimationProxy();

		/** 
		 * Rebuilds the internal proxy data according to the newly assigned skeleton and clips. This should be called
		 * whenever the animation skeleton changes.
		 *
		 * @param[in]		skeleton	New skeleton to assign to the proxy.
		 * @param[in, out]	clipInfos	Potentially new clip infos that will be used for rebuilding the proxy. Once the
		 *								method completes clip info layout and state indices will be populated for 
		 *								further use in the update*() methods.
		 *
		 * @note	Should be called from the sim thread when the caller is sure the animation thread is not using it.
		 */
		void rebuild(const SPtr<Skeleton>& skeleton, Vector<AnimationClipInfo>& clipInfos);

		/** 
		 * Rebuilds the internal proxy data according to the newly clips. This should be called whenever clips are added
		 * or removed, or clip layout indices change.
		 *
		 * @param[in, out]	clipInfos	New clip infos that will be used for rebuilding the proxy. Once the method completes
		 *								clip info layout and state indices will be populated for further use in the
		 *								update*() methods.
		 *
		 * @note	Should be called from the sim thread when the caller is sure the animation thread is not using it.
		 */
		void rebuild(Vector<AnimationClipInfo>& clipInfos);

		/** 
		 * Updates the proxy data with new information about the clips. Caller must guarantee that clip layout didn't 
		 * change since the last call to rebuild().
		 *
		 * @note	Should be called from the sim thread when the caller is sure the animation thread is not using it.
		 */
		void updateValues(const Vector<AnimationClipInfo>& clipInfos);

		/** 
		 * Updates the proxy data with new clip times. Caller must guarantee that clip layout didn't change since the last
		 * call to rebuild().
		 *
		 * @note	Should be called from the sim thread when the caller is sure the animation thread is not using it.
		 */
		void updateTime(const Vector<AnimationClipInfo>& clipInfos);

		UINT64 id;
		AnimationStateLayer* layers;
		UINT32 numLayers;
		SPtr<Skeleton> skeleton;

		// Evaluation results
		LocalSkeletonPose localPose;
		float* genericCurveOutputs;
	};

	/**
	 * Handles animation playback. Takes one or multiple animation clips as input and evaluates them every animation update
	 * tick depending on set properties. The evaluated data is used by the core thread for skeletal animation, by the sim
	 * thread for updating attached scene objects and bones (if skeleton is attached), or the data is made available for
	 * manual queries in the case of generic animation.
	 */
	class BS_CORE_EXPORT Animation : public CoreObject
	{
	public:
		~Animation();

		/** 
		 * Changes the skeleton which will the translation/rotation/scale animation values manipulate. If no skeleton is set
		 * the animation will only evaluate the generic curves, and the root translation/rotation/scale curves.
		 */
		void setSkeleton(const SPtr<Skeleton>& skeleton);

		/** 
		 * Changes the wrap mode for all active animations. Wrap mode determines what happens when animation reaches the 
		 * first or last frame. 
		 *
		 * @see	AnimWrapMode
		 */
		void setWrapMode(AnimWrapMode wrapMode);

		/** Changes the speed for all animations. The default value is 1.0f. Use negative values to play-back in reverse. */
		void setSpeed(float speed);

		/** 
		 * Plays the specified animation clip. 
		 *
		 * @param[in]	clip		Clip to play.
		 */
		void play(const HAnimationClip& clip);

		/**
		 * Plays the specified animation clip on top of the animation currently playing in the main layer. Multiple
		 * such clips can be playing at once, as long as you ensure each is given its own layer. Each animation can
		 * also have a weight that determines how much it influences the main animation.
		 *
		 * @param[in]	clip		Clip to additively blend. Must contain additive animation curves.
		 * @param[in]	weight		Determines how much of an effect will the blended animation have on the final output.
		 *							In range [0, 1].
		 * @param[in]	fadeLength	Applies the blend over a specified time period, increasing the weight as the time
		 *							passes. Set to zero to blend immediately. In seconds.
		 * @param[in]	layer		Layer to play the clip in. Multiple additive clips can be playing at once in separate
		 *							layers and each layer has its own weight.
		 */
		void blendAdditive(const HAnimationClip& clip, float weight, float fadeLength = 0.0f, UINT32 layer = 0);

		/**
		 * Plays a set of animation clips sequentially one after another, with an optional fade between them.
		 *
		 * @param[in]	info		Describes all animation clips to play.
		 */
		void blendSequential(const BlendSequentialInfo& info);

		/**
		 * Blend two animation clips between each other using linear interpolation. Unlike normal animations these
		 * animations are not advanced with the progress of time, and is instead expected the user manually changes the
		 * @p t parameter.
		 *
		 * @param[in]	info	Information about the clips to blend.
		 * @param[in]	t		Parameter that controls the blending, in range [0, 1]. t = 0 means left animation has full
		 *						influence, t = 1 means right animation has full influence.
		 */
		void blend1D(const Blend1DInfo& info, float t);

		/**
		 * Blend four animation clips between each other using bilinear interpolation. Unlike normal animations these
		 * animations are not advanced with the progress of time, and is instead expected the user manually changes the
		 * @p t parameter.
		 *
		 * @param[in]	info	Information about the clips to blend.
		 * @param[in]	t		Parameter that controls the blending, in range [(0, 0), (1, 1)]. t = (0, 0) means top left
		 *						animation has full influence, t = (0, 1) means top right animation has full influence,
		 *						t = (1, 0) means bottom left animation has full influence, t = (1, 1) means bottom right
		 *						animation has full influence.
		 */
		void blend2D(const Blend2DInfo& info, const Vector2& t);

		/**
		 * Fades the specified animation clip in, while fading other playing animation out, over the specified time
		 * period.
		 *
		 * @param[in]	clip		Clip to fade in.
		 * @param[in]	fadeLength	Determines the time period over which the fade occurs. In seconds.
		 */
		void crossFade(const HAnimationClip& clip, float fadeLength);

		/** 
		 * Stops playing all animations on the provided layer. Specify -1 to stop animation on the main layer 
		 * (non-additive animations). 
		 */
		void stop(UINT32 layer);

		/** Stops playing all animations. */
		void stopAll();
		
		/** Checks if any animation clips are currently playing. */
		bool isPlaying() const;

		/** 
		 * Retrieves detailed information about a currently playing animation clip. 
		 *
		 * @param[in]	clip	Clip to retrieve the information for.
		 * @param[out]	state	Animation clip state containing the requested information. Only valid if the method returns
		 *						true.
		 * @return				True if the state was found (animation clip is playing), false otherwise.
		 */
		bool getState(const HAnimationClip& clip, AnimationClipState& state);

		/**
		 * Changes the state of a playing animation clip. If animation clip is not currently playing the state change is
		 * ignored.
		 *
		 * @param[in]	clip	Clip to change the state for.
		 * @param[in]	state	New state of the animation (e.g. changing the time for seeking).
		 */
		void setState(const HAnimationClip& clip, AnimationClipState state);

		/** Creates a new empty Animation object. */
		static SPtr<Animation> create();

		/** @name Internal 
		 *  @{
		 */

		/** Returns the unique ID for this animation object. */
		UINT64 _getId() const { return mId; }

		/** @} */
	private:
		friend class AnimationManager;

		Animation();

		/** 
		 * Updates the animation proxy object based on the currently set skeleton, playing clips and dirty flags. 
		 *
		 * @param[in]	timeDelta	Seconds passed since the last call to this method.
		 */
		void updateAnimProxy(float timeDelta);

		/** 
		 * Registers a new animation in the specified layer, or returns an existing animation clip info if the animation is
		 * already registered. If @p stopExisting is true any existing animations in the layer will be stopped. Layout
		 * will be marked as dirty if any changes were made.
		 */
		AnimationClipInfo* addClip(const HAnimationClip& clip, UINT32 layer, bool stopExisting = true);

		UINT64 mId;
		AnimWrapMode mDefaultWrapMode;
		float mDefaultSpeed;
		AnimDirtyState mDirty;

		SPtr<Skeleton> mSkeleton;
		Vector<AnimationClipInfo> mClipInfos;

		// Animation thread only
		SPtr<AnimationProxy> mAnimProxy;
	};

	/** @} */
}