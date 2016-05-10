//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsOAPrerequisites.h"

namespace BansheeEngine
{
	/** @addtogroup OpenAudio
	 *  @{
	 */

	/** Used for reading .WAV audio files. */
	class OAWaveReader
	{
	public:
		OAWaveReader();

		bool open(const SPtr<DataStream>& stream, AudioFileInfo& info);
		void seek(UINT32 offset); // Offset in number of samples
		UINT32 read(UINT8* samples, UINT32 numSamples);

		static bool isValid(const SPtr<DataStream>& stream);
	private:
		bool parseHeader(AudioFileInfo& info);

		SPtr<DataStream> mStream;
		UINT32 mDataOffset;
		UINT32 mBytesPerSample;

		static const UINT32 MAIN_CHUNK_SIZE = 12;
	};

	/** @} */
}