//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGLHardwareBufferManager.h"
#include "BsGLVertexBuffer.h"
#include "BsGLIndexBuffer.h"
#include "BsGLGpuBuffer.h"
#include "BsHardwareBuffer.h"
#include "BsGLGpuParamBlockBuffer.h"
#include "BsRenderAPI.h"
#include "BsRenderAPICapabilities.h"

namespace BansheeEngine 
{
	SPtr<VertexBufferCore> GLHardwareBufferCoreManager::createVertexBufferInternal(UINT32 vertexSize, UINT32 numVerts, GpuBufferUsage usage, bool streamOut)
	{
		SPtr<GLVertexBufferCore> ret = bs_shared_ptr_new<GLVertexBufferCore>(vertexSize, numVerts, usage, streamOut);
		ret->_setThisPtr(ret);

		return ret;
	}

	SPtr<IndexBufferCore> GLHardwareBufferCoreManager::createIndexBufferInternal(IndexType itype, UINT32 numIndexes, GpuBufferUsage usage)
	{
		SPtr<GLIndexBufferCore> ret = bs_shared_ptr_new<GLIndexBufferCore>(itype, numIndexes, usage);
		ret->_setThisPtr(ret);

		return ret;
	}

	SPtr<GpuParamBlockBufferCore> GLHardwareBufferCoreManager::createGpuParamBlockBufferInternal(UINT32 size, GpuParamBlockUsage usage)
	{
		GLGpuParamBlockBufferCore* paramBlockBuffer = new (bs_alloc<GLGpuParamBlockBufferCore>()) GLGpuParamBlockBufferCore(size, usage);

		SPtr<GpuParamBlockBufferCore> paramBlockBufferPtr = bs_shared_ptr<GLGpuParamBlockBufferCore>(paramBlockBuffer);
		paramBlockBufferPtr->_setThisPtr(paramBlockBufferPtr);

		return paramBlockBufferPtr;
	}

	SPtr<GpuBufferCore> GLHardwareBufferCoreManager::createGpuBufferInternal(UINT32 elementCount, UINT32 elementSize,
		GpuBufferType type, GpuBufferUsage usage, bool randomGpuWrite, bool useCounter)
	{
		GLGpuBufferCore* buffer = new (bs_alloc<GLGpuBufferCore>()) GLGpuBufferCore(elementCount, elementSize, type, usage, randomGpuWrite, useCounter);

		SPtr<GpuBufferCore> bufferPtr = bs_shared_ptr<GLGpuBufferCore>(buffer);
		bufferPtr->_setThisPtr(bufferPtr);

		return bufferPtr;
	}

    GLenum GLHardwareBufferCoreManager::getGLUsage(GpuBufferUsage usage)
    {
        switch(usage)
        {
        case GBU_STATIC:
            return GL_STATIC_DRAW;
        case GBU_DYNAMIC:
            return GL_DYNAMIC_DRAW;
        default:
            return GL_DYNAMIC_DRAW;
        };
    }

    GLenum GLHardwareBufferCoreManager::getGLType(VertexElementType type)
    {
        switch(type)
        {
            case VET_FLOAT1:
            case VET_FLOAT2:
            case VET_FLOAT3:
            case VET_FLOAT4:
                return GL_FLOAT;
            case VET_SHORT1:
            case VET_SHORT2:
            case VET_SHORT3:
            case VET_SHORT4:
                return GL_SHORT;
            case VET_COLOR:
			case VET_COLOR_ABGR:
			case VET_COLOR_ARGB:
            case VET_UBYTE4:
                return GL_UNSIGNED_BYTE;
            default:
                return 0;
        };
    }
}
