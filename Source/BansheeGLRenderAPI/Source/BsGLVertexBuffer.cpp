//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGLHardwareBufferManager.h"
#include "BsGLVertexBuffer.h"
#include "BsGLVertexArrayObjectManager.h"
#include "BsRenderStats.h"
#include "BsException.h"

namespace BansheeEngine 
{
	GLVertexBufferCore::GLVertexBufferCore(UINT32 vertexSize, UINT32 numVertices, GpuBufferUsage usage, bool streamOut)
		:VertexBufferCore(vertexSize, numVertices, usage, streamOut)
    {

    }

	GLVertexBufferCore::~GLVertexBufferCore()
	{
		while (mVAObjects.size() > 0)
			GLVertexArrayObjectManager::instance().notifyBufferDestroyed(mVAObjects[0]);

		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_VertexBuffer);
	}

	void GLVertexBufferCore::initialize()
	{
		mBuffer = GLBuffer(GL_ARRAY_BUFFER, mSizeInBytes, mUsage);
		
		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_VertexBuffer);
		VertexBufferCore::initialize();
	}

	void GLVertexBufferCore::registerVAO(const GLVertexArrayObject& vao)
	{
		mVAObjects.push_back(vao);
	}

	void GLVertexBufferCore::unregisterVAO(const GLVertexArrayObject& vao)
	{
		auto iterFind = std::find(mVAObjects.begin(), mVAObjects.end(), vao);

		if (iterFind != mVAObjects.end())
			mVAObjects.erase(iterFind);
	}

	void* GLVertexBufferCore::lockImpl(UINT32 offset, UINT32 length, GpuLockOptions options)
    {
		return mBuffer.lock(offset, length, options);
    }

	void GLVertexBufferCore::unlockImpl()
    {
		mBuffer.unlock();
    }

	void GLVertexBufferCore::readData(UINT32 offset, UINT32 length, void* pDest)
    {
		mBuffer.readData(offset, length, pDest);
    }

	void GLVertexBufferCore::writeData(UINT32 offset, UINT32 length,
		const void* pSource, BufferWriteType writeFlags)
    {
		mBuffer.writeData(offset, length, pSource, writeFlags);
    }
}
