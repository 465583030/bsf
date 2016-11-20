//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsIndexBuffer.h"
#include "BsHardwareBufferManager.h"
#include "BsRenderAPI.h"

namespace bs 
{
	UINT32 calcIndexSize(IndexType type)
	{
		switch (type)
		{
		case IT_16BIT:
			return sizeof(unsigned short);
		default:
		case IT_32BIT:
			return sizeof(unsigned int);
		}
	}

	IndexBufferProperties::IndexBufferProperties(IndexType idxType, UINT32 numIndices)
		:mIndexType(idxType), mNumIndices(numIndices), mIndexSize(calcIndexSize(idxType))
	{ }

	IndexBufferCore::IndexBufferCore(const INDEX_BUFFER_DESC& desc, GpuDeviceFlags deviceMask)
		:HardwareBuffer(calcIndexSize(desc.indexType) * desc.numIndices), mProperties(desc.indexType, desc.numIndices)
	{ }

	SPtr<IndexBufferCore> IndexBufferCore::create(const INDEX_BUFFER_DESC& desc, GpuDeviceFlags deviceMask)
	{
		return HardwareBufferCoreManager::instance().createIndexBuffer(desc, deviceMask);
	}

    IndexBuffer::IndexBuffer(const INDEX_BUFFER_DESC& desc)
		:mProperties(desc.indexType, desc.numIndices), mUsage(desc.usage)
    {

	}

	SPtr<IndexBufferCore> IndexBuffer::getCore() const
	{
		return std::static_pointer_cast<IndexBufferCore>(mCoreSpecific);
	}

	SPtr<CoreObjectCore> IndexBuffer::createCore() const
	{
		INDEX_BUFFER_DESC desc;
		desc.indexType = mProperties.mIndexType;
		desc.numIndices = mProperties.mNumIndices;
		desc.usage = mUsage;

		return HardwareBufferCoreManager::instance().createIndexBufferInternal(desc);
	}

	SPtr<IndexBuffer> IndexBuffer::create(const INDEX_BUFFER_DESC& desc)
	{
		return HardwareBufferManager::instance().createIndexBuffer(desc);
	}
}