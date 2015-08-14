#pragma once

#include "BsGLPrerequisites.h"
#include "BsGpuProgram.h"

namespace BansheeEngine 
{
	/**
	 * @brief	GPU program compiled from GLSL and usable by OpenGL.
	 */
    class BS_RSGL_EXPORT GLSLGpuProgramCore : public GpuProgramCore
    {
	public:
		~GLSLGpuProgramCore();

		/**
		 * @copydoc	GpuProgramCore::isSupported
		 */
		bool isSupported() const override;

		/**
		 * @brief	Gets vertex declaration that determines which input attributes does the GPU
		 *			program expect (and which attributes will it retrieve from the bound vertex buffer). 
		 *			Only valid for vertex programs.
		 */
		const VertexDeclarationCore& getInputAttributes() const { return *mVertexDeclaration; }

		/**
		 * @brief	Gets internal OpenGL handle to the program.
		 */
		GLuint getGLHandle() const { return mGLHandle; }

		/**
		 * @brief	Gets an unique index for this GPU program. Each created GPU program is
		 *			assigned a unique index on creation.
		 */
		UINT32 getProgramID() const { return mProgramID; }

		/**
		 * @copydoc	GpuProgramCore::hasColumnMajorMatrices
		 */
		bool hasColumnMajorMatrices() const override { return true; }

	private:
		friend class GLSLProgramFactory;

		GLSLGpuProgramCore(const String& source, const String& entryPoint, GpuProgramType gptype, 
			GpuProgramProfile profile, bool isAdjacencyInfoRequired);

		/**
		 * @copydoc GpuProgramCore::initialize
		 */
		void initialize() override;

	private:
		UINT32 mProgramID;
		GLuint mGLHandle;

		SPtr<VertexDeclarationCore> mVertexDeclaration;
		
		static UINT32 mVertexShaderCount;
		static UINT32 mFragmentShaderCount;
		static UINT32 mGeometryShaderCount;
		static UINT32 mHullShaderCount;
		static UINT32 mDomainShaderCount;
    };
}
