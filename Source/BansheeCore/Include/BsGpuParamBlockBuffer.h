//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsCoreObject.h"

namespace BansheeEngine
{
	/** @addtogroup RenderAPI-Internal
	 *  @{
	 */

	/**
	 * Core thread version of a GpuParamBlockBuffer.
	 *
	 * @note	Core thread only.
	 */
	class BS_CORE_EXPORT GpuParamBlockBufferCore : public CoreObjectCore
	{
	public:
		GpuParamBlockBufferCore(UINT32 size, GpuParamBlockUsage usage);
		virtual ~GpuParamBlockBufferCore();

		/** Writes all of the specified data to the buffer. Data size must be the same size as the buffer. */
		virtual void writeToGPU(const UINT8* data) = 0;

		/**
		 * Copies data from the internal buffer to a pre-allocated array. Be aware this generally isn't a very fast 
		 * operation as reading from the GPU will most definitely involve a CPU-GPU sync point.
		 *
		 * @param[in,out]	data	Array where the data will be written to. Must be of getSize() bytes.
		 */
		virtual void readFromGPU(UINT8* data) const = 0;

		/** Flushes any cached data into the actual GPU buffer. */
		void flushToGPU();

		/**
		 * Write some data to the specified offset in the buffer. 
		 *
		 * @note	All values are in bytes. Actual hardware buffer update is delayed until rendering.
		 */
		void write(UINT32 offset, const void* data, UINT32 size);

		/**
		 * Read some data from the specified offset in the buffer.
		 *			
		 * @note	All values are in bytes. This reads from the cached CPU buffer and not directly from the GPU.
		 */
		void read(UINT32 offset, void* data, UINT32 size);

		/**
		 * Clear specified section of the buffer to zero.
		 *
		 * @note	All values are in bytes. Actual hardware buffer update is delayed until rendering.
		 */
		void zeroOut(UINT32 offset, UINT32 size);

		/**	Returns the size of the buffer in bytes. */
		UINT32 getSize() const { return mSize; }

		/** @copydoc HardwareBufferCoreManager::createGpuParamBlockBuffer */
		static SPtr<GpuParamBlockBufferCore> create(UINT32 size, GpuParamBlockUsage usage = GPBU_DYNAMIC);

	protected:
		/** @copydoc CoreObjectCore::syncToCore */
		virtual void syncToCore(const CoreSyncData& data)  override;

		GpuParamBlockUsage mUsage;
		UINT32 mSize;

		UINT8* mCachedData;
		bool mGPUBufferDirty;
	};

	/**
	 * Implementation of a GpuParamBlock buffer that doesn't use a GPU buffer for storage. Used with APIs that do not 
	 * support GPU parameter buffers.
	 */
	class BS_CORE_EXPORT GenericGpuParamBlockBufferCore : public GpuParamBlockBufferCore
	{
	public:
		GenericGpuParamBlockBufferCore(UINT32 size, GpuParamBlockUsage usage);
		~GenericGpuParamBlockBufferCore();

		/** @copydoc GpuParamBlockBufferCore::writeData */
		void writeToGPU(const UINT8* data) override;

		/** @copydoc GpuParamBlockBufferCore::readData */
		void readFromGPU(UINT8* data) const override;

	protected:
		UINT8* mData;

		/** @copydoc CoreObjectCore::initialize */
		virtual void initialize() override;
	};

	/** @} */
	/** @addtogroup RenderAPI
	 *  @{
	 */

	/**
	 * Represents a GPU parameter block buffer. Parameter block buffers are bound to GPU programs which then fetch 
	 * parameters from those buffers.
	 *
	 * Writing or reading from this buffer will translate directly to API calls that update the GPU.
	 * 			
	 * @note	Sim thread only.
	 */
	class BS_CORE_EXPORT GpuParamBlockBuffer : public CoreObject
	{
	public:
		GpuParamBlockBuffer(UINT32 size, GpuParamBlockUsage usage);
		virtual ~GpuParamBlockBuffer();

		/**
		 * Write some data to the specified offset in the buffer. 
		 *
		 * @note	All values are in bytes. Actual hardware buffer update is delayed until rendering.
		 */
		void write(UINT32 offset, const void* data, UINT32 size);

		/**
		 * Read some data from the specified offset in the buffer.
		 *			
		 * @note	All values are in bytes. This reads from the cached CPU buffer and not from the GPU.
		 */
		void read(UINT32 offset, void* data, UINT32 size);

		/**
		 * Clear specified section of the buffer to zero.
		 *
		 * @note	All values are in bytes. Actual hardware buffer update is delayed until rendering.
		 */
		void zeroOut(UINT32 offset, UINT32 size);

		/** Returns internal cached data of the buffer. */
		const UINT8* getCachedData() const { return mCachedData; }

		/**	Returns the size of the buffer in bytes. */
		UINT32 getSize() const { return mSize; }

		/**	Retrieves a core implementation of a GPU param block buffer usable only from the core thread. */
		SPtr<GpuParamBlockBufferCore> getCore() const;

		/** @copydoc HardwareBufferManager::createGpuParamBlockBuffer */
		static GpuParamBlockBufferPtr create(UINT32 size, GpuParamBlockUsage usage = GPBU_DYNAMIC);

	protected:
		/** @copydoc CoreObject::createCore */
		SPtr<CoreObjectCore> createCore() const override;

		/** @copydoc CoreObject::syncToCore */
		virtual CoreSyncData syncToCore(FrameAlloc* allocator) override;

		GpuParamBlockUsage mUsage;
		UINT32 mSize;
		UINT8* mCachedData;
	};

	/** @endcond */
	/** @} */
}