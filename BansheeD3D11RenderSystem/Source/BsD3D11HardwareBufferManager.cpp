#include "BsD3D11HardwareBufferManager.h"
#include "BsD3D11VertexBuffer.h"
#include "BsD3D11IndexBuffer.h"
#include "BsD3D11GpuBuffer.h"
#include "BsD3D11GpuParamBlockBuffer.h"
#include "BsGpuParamDesc.h"

namespace BansheeEngine
{
	D3D11HardwareBufferCoreManager::D3D11HardwareBufferCoreManager(D3D11Device& device)
		: mDevice(device)
	{ }

	SPtr<VertexBufferCore> D3D11HardwareBufferCoreManager::createVertexBufferInternal(UINT32 vertexSize,
		UINT32 numVerts, GpuBufferUsage usage, bool streamOut)
	{
		SPtr<D3D11VertexBufferCore> ret = bs_shared_ptr<D3D11VertexBufferCore>(mDevice, vertexSize, numVerts, usage, streamOut);
		ret->_setThisPtr(ret);

		return ret;
	}

	SPtr<IndexBufferCore> D3D11HardwareBufferCoreManager::createIndexBufferInternal(IndexType itype,
		UINT32 numIndexes, GpuBufferUsage usage)
	{
		SPtr<D3D11IndexBufferCore> ret = bs_shared_ptr<D3D11IndexBufferCore>(mDevice, itype, numIndexes, usage);
		ret->_setThisPtr(ret);

		return ret;
	}

	SPtr<GpuParamBlockBufferCore> D3D11HardwareBufferCoreManager::createGpuParamBlockBufferInternal(UINT32 size, GpuParamBlockUsage usage)
	{
		D3D11GpuParamBlockBufferCore* paramBlockBuffer = new (bs_alloc<D3D11GpuParamBlockBufferCore, GenAlloc>()) D3D11GpuParamBlockBufferCore(size, usage);

		SPtr<GpuParamBlockBufferCore> paramBlockBufferPtr = bs_shared_ptr<D3D11GpuParamBlockBufferCore, GenAlloc>(paramBlockBuffer);
		paramBlockBufferPtr->_setThisPtr(paramBlockBufferPtr);

		return paramBlockBufferPtr;
	}

	SPtr<GpuBufferCore> D3D11HardwareBufferCoreManager::createGpuBufferInternal(UINT32 elementCount, UINT32 elementSize,
		GpuBufferType type, GpuBufferUsage usage, bool randomGpuWrite, bool useCounter)
	{
		D3D11GpuBufferCore* buffer = new (bs_alloc<D3D11GpuBufferCore, GenAlloc>()) D3D11GpuBufferCore(elementCount, elementSize, type, usage, randomGpuWrite, useCounter);

		SPtr<D3D11GpuBufferCore> bufferPtr = bs_shared_ptr<D3D11GpuBufferCore, GenAlloc>(buffer);
		bufferPtr->_setThisPtr(bufferPtr);

		return bufferPtr;
	}
}