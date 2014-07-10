//__________________________ Banshee Project - A modern game development toolkit _________________________________//
//_____________________________________ www.banshee-project.com __________________________________________________//
//________________________ Copyright (c) 2014 Marko Pintera. All rights reserved. ________________________________//
#include "BsD3D11IndexBuffer.h"
#include "BsD3D11Device.h"
#include "BsRenderStats.h"

namespace BansheeEngine
{
	D3D11IndexBuffer::D3D11IndexBuffer(D3D11Device& device, IndexType idxType, UINT32 numIndexes, 
		GpuBufferUsage usage, bool useSystemMem)
		:IndexBuffer(idxType, numIndexes, usage, useSystemMem), mDevice(device), mBuffer(nullptr)
	{

	}

	D3D11IndexBuffer::~D3D11IndexBuffer()
	{
		
	}

	void D3D11IndexBuffer::initialize_internal()
	{
		mBuffer = bs_new<D3D11HardwareBuffer, PoolAlloc>(D3D11HardwareBuffer::BT_INDEX, mUsage, 1, mSizeInBytes, std::ref(mDevice), mSystemMemory);

		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_IndexBuffer);
		IndexBuffer::initialize_internal();
	}

	void D3D11IndexBuffer::destroy_internal()
	{
		if(mBuffer != nullptr)
			bs_delete<PoolAlloc>(mBuffer) ;

		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_IndexBuffer);

		IndexBuffer::destroy_internal();
	}

	void* D3D11IndexBuffer::lockImpl(UINT32 offset, UINT32 length, GpuLockOptions options)
	{
#if BS_PROFILING_ENABLED
		if (options == GBL_READ_ONLY || options == GBL_READ_WRITE)
		{
			BS_INC_RENDER_STAT_CAT(ResRead, RenderStatObject_IndexBuffer);
		}

		if (options == GBL_READ_WRITE || options == GBL_WRITE_ONLY || options == GBL_WRITE_ONLY_DISCARD || options == GBL_WRITE_ONLY_NO_OVERWRITE)
		{
			BS_INC_RENDER_STAT_CAT(ResWrite, RenderStatObject_IndexBuffer);
		}
#endif

		return mBuffer->lock(offset, length, options);
	}

	void D3D11IndexBuffer::unlockImpl()
	{
		mBuffer->unlock();
	}

	void D3D11IndexBuffer::readData(UINT32 offset, UINT32 length, void* pDest)
	{
		mBuffer->readData(offset, length, pDest);

		BS_INC_RENDER_STAT_CAT(ResRead, RenderStatObject_IndexBuffer);
	}

	void D3D11IndexBuffer::writeData(UINT32 offset, UINT32 length, const void* pSource, BufferWriteType writeFlags)
	{
		mBuffer->writeData(offset, length, pSource, writeFlags);

		BS_INC_RENDER_STAT_CAT(ResWrite, RenderStatObject_IndexBuffer);
	}

	void D3D11IndexBuffer::copyData(HardwareBuffer& srcBuffer, UINT32 srcOffset, 
		UINT32 dstOffset, UINT32 length, bool discardWholeBuffer)
	{
		mBuffer->copyData(srcBuffer, srcOffset, dstOffset, length, discardWholeBuffer);
	}
}