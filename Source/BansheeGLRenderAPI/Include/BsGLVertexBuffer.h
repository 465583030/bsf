//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsGLPrerequisites.h"
#include "BsVertexBuffer.h"
#include "BsGLVertexArrayObjectManager.h"

namespace BansheeEngine 
{
	/** @addtogroup GL
	 *  @{
	 */

	/**	OpenGL implementation of a vertex buffer. */
    class BS_RSGL_EXPORT GLVertexBufferCore : public VertexBufferCore
    {
    public:
		GLVertexBufferCore(UINT32 vertexSize, UINT32 numVertices, GpuBufferUsage usage, bool streamOut);
		~GLVertexBufferCore();

		/** @copydoc VertexBufferCore::readData */
        void readData(UINT32 offset, UINT32 length, void* pDest) override;

		/** @copydoc VertexBufferCore::writeData */
        void writeData(UINT32 offset, UINT32 length, const void* pSource, 
			BufferWriteType writeFlags = BufferWriteType::Normal) override;

		/**	Returns internal OpenGL buffer ID. */
        GLuint getGLBufferId() const { return mBufferId; }

		/**	Registers a new VertexArrayObject that uses this vertex buffer. */
		void registerVAO(const GLVertexArrayObject& vao);

		/**	Unregisters a VAO from this vertex buffer. Does not destroy it. */
		void unregisterVAO(const GLVertexArrayObject& vao);

	protected:
		/** @copydoc VertexBufferCore::initialize */
		void initialize() override;

		/** @copydoc VertexBufferCore::lockImpl */
		void* lockImpl(UINT32 offset, UINT32 length, GpuLockOptions options) override;

		/** @copydod VertexBufferCore::unlockImpl */
		void unlockImpl() override;

	private:
		GLuint mBufferId;
		bool mZeroLocked;

		Vector<GLVertexArrayObject> mVAObjects;
    };

	/** @} */
}