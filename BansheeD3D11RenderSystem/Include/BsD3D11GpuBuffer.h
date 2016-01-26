#pragma once

#include "BsD3D11Prerequisites.h"
#include "BsGpuBuffer.h"

namespace BansheeEngine 
{
	/**
	 * @brief	DirectX 11 implementation of a generic GPU buffer.
	 */
	class BS_D3D11_EXPORT D3D11GpuBuffer : public GpuBuffer
    {
    public:
        ~D3D11GpuBuffer();

		/**
		 * @copydoc GenericBuffer::lockImpl
		 */
		virtual void* lock(UINT32 offset, UINT32 length, GpuLockOptions options);

		/**
		 * @copydoc GenericBuffer::unlockImpl
		 */
		virtual void unlock();

		/**
		 * @copydoc GenericBuffer::readData
		 */
        virtual void readData(UINT32 offset, UINT32 length, void* pDest);

		/**
		 * @copydoc GenericBuffer::writeData
		 */
        virtual void writeData(UINT32 offset, UINT32 length, const void* pSource,
			BufferWriteType writeFlags = BufferWriteType::Normal);

		/**
		 * @copydoc GenericBuffer::copyData
		 */
		void copyData(GpuBuffer& srcBuffer, UINT32 srcOffset, 
			UINT32 dstOffset, UINT32 length, bool discardWholeBuffer = false);

		/**
		 * @brief	Returns the internal DX11 GPU buffer object.
		 */
		ID3D11Buffer* getDX11Buffer() const;

	protected:
		friend class D3D11HardwareBufferManager;

		/**
		 * @copydoc	GpuBuffer::GpuBuffer
		 */
		D3D11GpuBuffer(UINT32 elementCount, UINT32 elementSize, GpuBufferType type, GpuBufferUsage usage, 
			bool randomGpuWrite = false, bool useCounter = false);

		/**
		 * @copydoc GpuBuffer::createView
		 */
		virtual GpuBufferView* createView();

		/**
		 * @copydoc GpuBuffer::destroyView
		 */
		virtual void destroyView(GpuBufferView* view);

		/**
		 * @copydoc GpuBuffer::initialize_internal
		 */
		void initialize_internal();	
		
		/**
		 * @copydoc GpuBuffer::destroy_internal
		 */
		void destroy_internal();

	private:
		D3D11HardwareBuffer* mBuffer;
    };
}
