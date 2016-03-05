//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsD3D11Prerequisites.h"
#include "BsIndexBuffer.h"
#include "BsD3D11HardwareBuffer.h"

namespace BansheeEngine
{
	/**
	 * @brief	DirectX 11 implementation of an index buffer.
	 */
	class BS_D3D11_EXPORT D3D11IndexBufferCore : public IndexBufferCore
	{
	public:
		D3D11IndexBufferCore(D3D11Device& device, IndexType idxType, UINT32 numIndexes, GpuBufferUsage usage);

		~D3D11IndexBufferCore();

		/**
		 * @copydoc IndexBufferCore::readData
		 */
		void readData(UINT32 offset, UINT32 length, void* pDest) override;

		/**
		 * @copydoc IndexBufferCore::writeData
		 */
		void writeData(UINT32 offset, UINT32 length, const void* pSource, BufferWriteType writeFlags = BufferWriteType::Normal) override;

		/**
		 * @copydoc IndexBufferCore::copyData
		 */
		void copyData(HardwareBuffer& srcBuffer, UINT32 srcOffset, UINT32 dstOffset, UINT32 length, bool discardWholeBuffer = false) override;

		/**
		 * @brief	Gets the internal DX11 index buffer object.
		 */
		ID3D11Buffer* getD3DIndexBuffer() const { return mBuffer->getD3DBuffer(); }		

	protected:
		/**
		 * @copydoc IndexBufferCore::lockImpl
		 */
		void* lockImpl(UINT32 offset, UINT32 length, GpuLockOptions options) override;

		/**
		 * @copydoc IndexBufferCore::unlockImpl
		 */
		void unlockImpl() override;

		/**
		 * @copydoc IndexBufferCore::initialize
		 */
		void initialize() override;

		D3D11HardwareBuffer* mBuffer;
		D3D11Device& mDevice;
	};
}