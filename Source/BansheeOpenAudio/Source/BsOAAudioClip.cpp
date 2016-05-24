//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsOAAudioClip.h"
#include "BsOAOggVorbisWriter.h"
#include "BsOAOggVorbisReader.h"
#include "BsDataStream.h"
#include "BsOAAudio.h"
#include "AL/al.h"

namespace BansheeEngine
{
	OAAudioClip::OAAudioClip(const SPtr<DataStream>& samples, UINT32 streamSize, UINT32 numSamples, const AUDIO_CLIP_DESC& desc)
		:AudioClip(samples, streamSize, numSamples, desc), mNeedsDecompression(false), mBufferId((UINT32)-1), mSourceStreamSize(0)
	{ }

	OAAudioClip::~OAAudioClip()
	{
		if (mBufferId != (UINT32)-1)
			alDeleteBuffers(1, &mBufferId);
	}

	void OAAudioClip::initialize()
	{
		{
			Lock lock(mMutex); // Needs to be called even if stream data is null, to ensure memory fence is added so the
							   // other thread sees properly initialized AudioClip members

			AudioFileInfo info;
			info.bitDepth = mDesc.bitDepth;
			info.numChannels = mDesc.numChannels;
			info.numSamples = mNumSamples;
			info.sampleRate = mDesc.frequency;

			// If we need to keep source data, read everything into memory and keep a copy
			if (mKeepSourceData)
			{
				mStreamData->seek(mStreamOffset);

				UINT8* sampleBuffer = (UINT8*)bs_alloc(mStreamSize);
				mStreamData->read(sampleBuffer, mStreamSize);

				mSourceStreamData = bs_shared_ptr_new<MemoryDataStream>(sampleBuffer, mStreamSize);
				mSourceStreamSize = mStreamSize;
			}

			if(mDesc.readMode == AudioReadMode::LoadDecompressed && mDesc.format == AudioFormat::VORBIS)
			{
				// Read all uncompressed data into memory
				SPtr<DataStream> stream;
				UINT32 offset = 0;
				if (mSourceStreamData != nullptr) // If it's already loaded in memory, use it directly
					stream = mSourceStreamData;
				else
				{
					stream = mStreamData;
					offset = mStreamOffset;
				}

				stream->seek(offset);

				// Decompress from Ogg
				OAOggVorbisReader reader;
				if (reader.open(stream, info))
				{
					UINT32 bufferSize = info.numSamples * info.bitDepth;
					UINT8* sampleBuffer = (UINT8*)bs_stack_alloc(bufferSize);

					reader.read(sampleBuffer, info.numSamples);

					alGenBuffers(1, &mBufferId);
					gOAAudio()._writeToOpenALBuffer(mBufferId, sampleBuffer, info);

					mStreamData = nullptr;
					mStreamOffset = 0;
					mStreamSize = 0;

					bs_stack_free(sampleBuffer);
				}
				else
					LOGERR("Failed decompressing AudioClip stream.");
			}
			else if(mDesc.readMode == AudioReadMode::LoadCompressed)
			{
				// If reading from file, make a copy of data in memory, otherwise just take ownership of the existing buffer
				if (mStreamData->isFile()) 
				{
					if (mSourceStreamData != nullptr) // If it's already loaded in memory, use it directly
						mStreamData = mSourceStreamData;
					else
					{
						UINT8* data = (UINT8*)bs_alloc(mStreamSize);

						mStreamData->seek(mStreamOffset);
						mStreamData->read(data, mStreamSize);

						mStreamData = bs_shared_ptr_new<MemoryDataStream>(data, mStreamSize);
					}

					mStreamOffset = 0;
				}
			}

			if (mDesc.format == AudioFormat::VORBIS && mDesc.readMode != AudioReadMode::LoadDecompressed)
			{
				mNeedsDecompression = true;

				if (mStreamData != nullptr)
				{
					if (!mVorbisReader.open(mStreamData, info))
						LOGERR("Failed decompressing AudioClip stream.");
				}
			}

			if (mStreamData != nullptr)
				mStreamData->seek(mStreamOffset);
		}

		AudioClip::initialize();
	}

	void OAAudioClip::getSamples(UINT8* samples, UINT32 offset, UINT32 count) const
	{
		Lock lock(mMutex);

		// Try to read from normal stream, and if that fails read from in-memory stream if it exists
		if (mStreamData == nullptr)
		{
			if (mNeedsDecompression)
			{
				mVorbisReader.seek(offset);
				mVorbisReader.read(samples, count);
			}
			else
			{
				UINT32 bytesPerSample = mDesc.bitDepth / 8;
				UINT32 size = count * bytesPerSample;
				UINT32 streamOffset = mStreamOffset + offset * bytesPerSample;

				mStreamData->seek(streamOffset);
				mStreamData->read(samples, size);
			}

			return;
		}

		if (mSourceStreamData != nullptr)
		{
			assert(!mNeedsDecompression); // Normal stream must exist if decompressing

			UINT32 bytesPerSample = mDesc.bitDepth / 8;
			UINT32 size = count * bytesPerSample;
			UINT32 streamOffset = offset * bytesPerSample;

			mSourceStreamData->seek(streamOffset);
			mSourceStreamData->read(samples, size);
			return;
		}

		LOGWRN("Attempting to read samples while sample data is not available.");
	}

	SPtr<DataStream> OAAudioClip::getSourceStream(UINT32& size)
	{
		Lock lock(mMutex);

		size = mSourceStreamSize;
		mSourceStreamData->seek(0);

		return mSourceStreamData;
	}
}