#include "BsD3D11VertexBuffer.h"
#include "BsD3D11Device.h"
#include "BsRenderStats.h"

namespace BansheeEngine
{
	D3D11VertexBufferCore::D3D11VertexBufferCore(D3D11Device& device, UINT32 vertexSize, UINT32 numVertices, GpuBufferUsage usage, bool streamOut)
		:VertexBufferCore(vertexSize, numVertices, usage, streamOut), mDevice(device), mStreamOut(streamOut), mBuffer(nullptr)
	{ }

	D3D11VertexBufferCore::~D3D11VertexBufferCore()
	{
		if (mBuffer != nullptr)
			bs_delete<PoolAlloc>(mBuffer);

		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_VertexBuffer);
	}

	void* D3D11VertexBufferCore::lockImpl(UINT32 offset, UINT32 length, GpuLockOptions options)
	{
#if BS_PROFILING_ENABLED
		if (options == GBL_READ_ONLY || options == GBL_READ_WRITE)
		{
			BS_INC_RENDER_STAT_CAT(ResRead, RenderStatObject_VertexBuffer);
		}

		if (options == GBL_READ_WRITE || options == GBL_WRITE_ONLY || options == GBL_WRITE_ONLY_DISCARD || options == GBL_WRITE_ONLY_NO_OVERWRITE)
		{
			BS_INC_RENDER_STAT_CAT(ResWrite, RenderStatObject_VertexBuffer);
		}
#endif

		return mBuffer->lock(offset, length, options);
	}

	void D3D11VertexBufferCore::unlockImpl()
	{
		mBuffer->unlock();
	}

	void D3D11VertexBufferCore::readData(UINT32 offset, UINT32 length, void* dest)
	{
		mBuffer->readData(offset, length, dest);
		BS_INC_RENDER_STAT_CAT(ResRead, RenderStatObject_VertexBuffer);
	}

	void D3D11VertexBufferCore::writeData(UINT32 offset, UINT32 length, const void* source, BufferWriteType writeFlags)
	{
		mBuffer->writeData(offset, length, source, writeFlags);
		BS_INC_RENDER_STAT_CAT(ResWrite, RenderStatObject_VertexBuffer);
	}

	void D3D11VertexBufferCore::copyData(HardwareBuffer& srcBuffer, UINT32 srcOffset,
		UINT32 dstOffset, UINT32 length, bool discardWholeBuffer)
	{
		mBuffer->copyData(srcBuffer, srcOffset, dstOffset, length, discardWholeBuffer);
	}

	void D3D11VertexBufferCore::initialize()
	{
		mBuffer = bs_new<D3D11HardwareBuffer, PoolAlloc>(D3D11HardwareBuffer::BT_VERTEX, 
			mUsage, 1, mSizeInBytes, std::ref(mDevice), mSystemMemory, mStreamOut);

		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_VertexBuffer);
		VertexBufferCore::initialize();
	}
}