//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGLIndexBuffer.h"
#include "BsGLHardwareBufferManager.h"
#include "BsRenderStats.h"
#include "BsException.h"

namespace BansheeEngine 
{
	GLIndexBufferCore::GLIndexBufferCore(const INDEX_BUFFER_DESC& desc, GpuDeviceFlags deviceMask)
		:IndexBufferCore(desc, deviceMask)
	{
		assert((deviceMask == GDF_DEFAULT || deviceMask == GDF_PRIMARY) && "Multiple GPUs not supported natively on OpenGL.");
	}

	GLIndexBufferCore::~GLIndexBufferCore()
	{
		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_IndexBuffer);
	}

	void GLIndexBufferCore::initialize()
	{
		mBuffer.initialize(GL_ELEMENT_ARRAY_BUFFER, mSizeInBytes, mUsage);

		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_IndexBuffer);
		IndexBufferCore::initialize();
	}

	void* GLIndexBufferCore::map(UINT32 offset, UINT32 length, GpuLockOptions options, UINT32 syncMask)
	{
		return mBuffer.lock(offset, length, options);
	}

	void GLIndexBufferCore::unmap()
	{
		mBuffer.unlock();
	}

	void GLIndexBufferCore::readData(UINT32 offset, UINT32 length, void* pDest, UINT32 syncMask)
	{
		mBuffer.readData(offset, length, pDest);
	}

	void GLIndexBufferCore::writeData(UINT32 offset, UINT32 length,
		const void* pSource, BufferWriteType writeFlags, UINT32 syncMask)
	{
		mBuffer.writeData(offset, length, pSource, writeFlags);
	}
}
