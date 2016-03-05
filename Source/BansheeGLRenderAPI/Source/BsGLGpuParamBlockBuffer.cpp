//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGLGpuParamBlockBuffer.h"
#include "BsRenderStats.h"
#include "BsException.h"

namespace BansheeEngine
{
	GLGpuParamBlockBufferCore::GLGpuParamBlockBufferCore(UINT32 size, GpuParamBlockUsage usage)
		:GpuParamBlockBufferCore(size, usage), mGLHandle(0)
	{
	}

	GLGpuParamBlockBufferCore::~GLGpuParamBlockBufferCore()
	{
		glDeleteBuffers(1, &mGLHandle);

		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_GpuParamBuffer);
	}

	void GLGpuParamBlockBufferCore::initialize()
	{
		glGenBuffers(1, &mGLHandle);
		glBindBuffer(GL_UNIFORM_BUFFER, mGLHandle);
		if(mUsage == GPBU_STATIC)
			glBufferData(GL_UNIFORM_BUFFER, mSize, nullptr, GL_STATIC_DRAW);
		else if(mUsage == GPBU_DYNAMIC)
			glBufferData(GL_UNIFORM_BUFFER, mSize, nullptr, GL_DYNAMIC_DRAW);
		else
			BS_EXCEPT(InternalErrorException, "Invalid gpu param block usage.");

		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_GpuParamBuffer);
		GpuParamBlockBufferCore::initialize();
	}

	void GLGpuParamBlockBufferCore::writeToGPU(const UINT8* data)
	{
		glBindBuffer(GL_UNIFORM_BUFFER, mGLHandle);
		glBufferSubData(GL_UNIFORM_BUFFER, 0 , mSize, data);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		BS_INC_RENDER_STAT_CAT(ResWrite, RenderStatObject_GpuParamBuffer);
	}

	void GLGpuParamBlockBufferCore::readFromGPU(UINT8* data) const
	{
		glBindBuffer(GL_UNIFORM_BUFFER, mGLHandle);
		glGetBufferSubData(GL_UNIFORM_BUFFER, 0 , mSize, (GLvoid*)data);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		BS_INC_RENDER_STAT_CAT(ResRead, RenderStatObject_GpuParamBuffer);
	}
}