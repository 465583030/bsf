//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsFMODPrerequisites.h"
#include "BsAudioClip.h"
#include <fmod.hpp>

namespace BansheeEngine
{
	/** @addtogroup FMOD
	 *  @{
	 */
	
	/** FMOD implementation of an AudioClip. */
	class FMODAudioClip : public AudioClip
	{
	public:
		FMODAudioClip(const SPtr<DataStream>& samples, UINT32 streamSize, UINT32 numSamples, const AUDIO_CLIP_DESC& desc);
		virtual ~FMODAudioClip();

		/** 
		 * Creates a new streaming sound. Only valid if the clip was created with AudioReadMode::Stream. Caller is
		 * responsible for releasing the sound.
		 */
		FMOD::Sound* createStreamingSound() const;

		/** Returns FMOD sound representing this clip. Only valid for non-streaming clips. */
		FMOD::Sound* getSound() const { return mSound; }

	protected:
		/** @copydoc Resource::initialize */
		void initialize() override;

		/** @copydoc AudioClip::getSourceFormatData */
		SPtr<DataStream> getSourceStream(UINT32& size) override;

		FMOD::Sound* mSound;

		// These streams exist to save original audio data in case it's needed later (usually for saving with the editor, or
		// manual data manipulation). In normal usage (in-game) these will be null so no memory is wasted.
		SPtr<DataStream> mSourceStreamData;
		UINT32 mSourceStreamSize;
	};

	/** @} */
}