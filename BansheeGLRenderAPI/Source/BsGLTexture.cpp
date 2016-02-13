//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGLTexture.h"
#include "BsGLSupport.h"
#include "BsGLPixelFormat.h"
#include "BsGLPixelBuffer.h"
#include "BsException.h"
#include "BsBitwise.h"
#include "BsCoreThread.h"
#include "BsTextureManager.h"
#include "BsGLRenderTexture.h"
#include "BsRenderStats.h"

namespace BansheeEngine 
{
	GLTextureCore::GLTextureCore(GLSupport& support, TextureType textureType, UINT32 width, UINT32 height, UINT32 depth, 
		UINT32 numMipmaps, PixelFormat format, int usage, bool hwGamma, UINT32 multisampleCount, const PixelDataPtr& initialData)
		: TextureCore(textureType, width, height, depth, numMipmaps, format, usage, hwGamma, multisampleCount, initialData),
		mTextureID(0), mGLSupport(support), mGLFormat(0)
    { }

	GLTextureCore::~GLTextureCore()
    { 
		mSurfaceList.clear();
		glDeleteTextures(1, &mTextureID);

		clearBufferViews();

		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_Texture);
	}

	void GLTextureCore::initialize()
	{
		UINT32 width = mProperties.getWidth();
		UINT32 height = mProperties.getHeight();
		UINT32 depth = mProperties.getDepth();
		TextureType texType = mProperties.getTextureType();
		PixelFormat pixFormat = mProperties.getFormat();
		int usage = mProperties.getUsage();
		UINT32 numMips = mProperties.getNumMipmaps();

		// Check requested number of mipmaps
		UINT32 maxMips = PixelUtil::getMaxMipmaps(width, height, depth, mProperties.getFormat());
		if (numMips > maxMips)
			BS_EXCEPT(InvalidParametersException, "Invalid number of mipmaps. Maximum allowed is: " + toString(maxMips));

		if ((usage & TU_RENDERTARGET) != 0)
		{
			if (texType != TEX_TYPE_2D)
				BS_EXCEPT(NotImplementedException, "Only 2D render targets are supported at the moment");
		}

		if ((usage & TU_DEPTHSTENCIL) != 0)
		{
			if (texType != TEX_TYPE_2D)
				BS_EXCEPT(NotImplementedException, "Only 2D depth stencil targets are supported at the moment");

			if (!PixelUtil::isDepth(pixFormat))
				BS_EXCEPT(NotImplementedException, "Supplied format is not a depth stencil format. Format: " + toString(pixFormat));
		}

		// Generate texture handle
		glGenTextures(1, &mTextureID);

		// Set texture type
		glBindTexture(getGLTextureTarget(), mTextureID);

		// This needs to be set otherwise the texture doesn't get rendered
		glTexParameteri(getGLTextureTarget(), GL_TEXTURE_MAX_LEVEL, numMips);

		// Allocate internal buffer so that glTexSubImageXD can be used
		mGLFormat = GLPixelUtil::getClosestGLInternalFormat(pixFormat, mProperties.isHardwareGammaEnabled());

		if (PixelUtil::isCompressed(pixFormat))
		{
			if((usage & TU_RENDERTARGET) != 0)
				BS_EXCEPT(InvalidParametersException, "Cannot use a compressed format for a render target.");

			if ((usage & TU_DEPTHSTENCIL) != 0)
				BS_EXCEPT(InvalidParametersException, "Cannot use a compressed format for a depth stencil target.");
		}

		UINT32 sampleCount = mProperties.getMultisampleCount();
		if ((usage & TU_RENDERTARGET) != 0 && mProperties.getTextureType() == TEX_TYPE_2D && sampleCount > 1)
		{
			glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, sampleCount, mGLFormat, width, height, GL_FALSE);
		}
		else if ((usage & TU_DEPTHSTENCIL) != 0)
		{
			if (sampleCount > 1)
			{
				glTexImage2DMultisample(GL_TEXTURE_2D, sampleCount, mGLFormat,
					width, height, GL_FALSE);
			}
			else
			{
				GLenum depthStencilFormat = GLPixelUtil::getDepthStencilTypeFromFormat(pixFormat);

				glTexImage2D(GL_TEXTURE_2D, 0, mGLFormat,
					width, height, 0, 
					GL_DEPTH_STENCIL, depthStencilFormat, nullptr);
			}
		}
		else
		{
			GLenum baseFormat = GLPixelUtil::getGLOriginFormat(pixFormat);
			GLenum baseDataType = GLPixelUtil::getGLOriginDataType(pixFormat);

			// Run through this process to pre-generate mipmap pyramid
			for (UINT32 mip = 0; mip <= numMips; mip++)
			{
				switch (texType)
				{
				case TEX_TYPE_1D:
					glTexImage1D(GL_TEXTURE_1D, mip, mGLFormat, width, 0,
						baseFormat, baseDataType, nullptr);

					break;
				case TEX_TYPE_2D:
					glTexImage2D(GL_TEXTURE_2D, mip, mGLFormat,
						width, height, 0, baseFormat, baseDataType, nullptr);
					break;
				case TEX_TYPE_3D:
					glTexImage3D(GL_TEXTURE_3D, mip, mGLFormat, width, height,
						depth, 0, baseFormat, baseDataType, nullptr);
					break;
				case TEX_TYPE_CUBE_MAP:
					for(UINT32 face = 0; face < 6; face++) 
					{
						glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, mip, mGLFormat,
							width, height, 0, baseFormat, baseDataType, nullptr);
					}
					break;
				};

				if(width > 1)
					width = width/2;

				if(height > 1)
					height = height/2;

				if(depth > 1)	
					depth = depth/2;
			}
		}

		createSurfaceList();

		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_Texture);
		TextureCore::initialize();
	}

    GLenum GLTextureCore::getGLTextureTarget() const
    {
		switch (mProperties.getTextureType())
        {
            case TEX_TYPE_1D:
                return GL_TEXTURE_1D;
            case TEX_TYPE_2D:
				if (mProperties.getMultisampleCount() > 1)
					return GL_TEXTURE_2D_MULTISAMPLE;
				else
					return GL_TEXTURE_2D;
            case TEX_TYPE_3D:
                return GL_TEXTURE_3D;
            case TEX_TYPE_CUBE_MAP:
                return GL_TEXTURE_CUBE_MAP;
            default:
                return 0;
        };
    }

	GLuint GLTextureCore::getGLID() const
	{
		THROW_IF_NOT_CORE_THREAD;

		return mTextureID;
	}

	PixelData GLTextureCore::lockImpl(GpuLockOptions options, UINT32 mipLevel, UINT32 face)
	{
		if (mProperties.getMultisampleCount() > 1)
			BS_EXCEPT(InvalidStateException, "Multisampled textures cannot be accessed from the CPU directly.");

		if(mLockedBuffer != nullptr)
			BS_EXCEPT(InternalErrorException, "Trying to lock a buffer that's already locked.");

		UINT32 mipWidth = std::max(1u, mProperties.getWidth() >> mipLevel);
		UINT32 mipHeight = std::max(1u, mProperties.getHeight() >> mipLevel);
		UINT32 mipDepth = std::max(1u, mProperties.getDepth() >> mipLevel);

		PixelData lockedArea(mipWidth, mipHeight, mipDepth, mProperties.getFormat());

		mLockedBuffer = getBuffer(face, mipLevel);
		lockedArea.setExternalBuffer((UINT8*)mLockedBuffer->lock(options));

		return lockedArea;
	}

	void GLTextureCore::unlockImpl()
	{
		if(mLockedBuffer == nullptr)
			BS_EXCEPT(InternalErrorException, "Trying to unlock a buffer that's not locked.");

		mLockedBuffer->unlock();
		mLockedBuffer = nullptr;
	}


	void GLTextureCore::readData(PixelData& dest, UINT32 mipLevel, UINT32 face)
	{
		if (mProperties.getMultisampleCount() > 1)
			BS_EXCEPT(InvalidStateException, "Multisampled textures cannot be accessed from the CPU directly.");

		getBuffer(face, mipLevel)->download(dest);
	}

	void GLTextureCore::writeData(const PixelData& src, UINT32 mipLevel, UINT32 face, bool discardWholeBuffer)
	{
		if (mProperties.getMultisampleCount() > 1)
			BS_EXCEPT(InvalidStateException, "Multisampled textures cannot be accessed from the CPU directly.");

		getBuffer(face, mipLevel)->upload(src, src.getExtents());
	}

	void GLTextureCore::copyImpl(UINT32 srcFace, UINT32 srcMipLevel, UINT32 destFace, UINT32 destMipLevel, const SPtr<TextureCore>& target)
	{
		size_t numMips = std::min(mProperties.getNumMipmaps(), target->getProperties().getNumMipmaps());

		GLTextureCore* destTex = static_cast<GLTextureCore*>(target.get());
		GLTextureBuffer *src = static_cast<GLTextureBuffer*>(getBuffer(srcFace, srcMipLevel).get());

		destTex->getBuffer(destFace, destMipLevel)->blitFromTexture(src);
	}

	void GLTextureCore::createSurfaceList()
	{
		mSurfaceList.clear();
		
		for (UINT32 face = 0; face < mProperties.getNumFaces(); face++)
		{
			for (UINT32 mip = 0; mip <= mProperties.getNumMipmaps(); mip++)
			{
                GLPixelBuffer *buf = bs_new<GLTextureBuffer>(getGLTextureTarget(), mTextureID, face, mip,
					static_cast<GpuBufferUsage>(mProperties.getUsage()), 
					mProperties.isHardwareGammaEnabled(), mProperties.getMultisampleCount());

				mSurfaceList.push_back(bs_shared_ptr<GLPixelBuffer>(buf));
                if(buf->getWidth() == 0 || buf->getHeight() == 0 || buf->getDepth() == 0)
                {
					BS_EXCEPT(RenderingAPIException, 
                        "Zero sized texture surface on texture face "
						+ toString(face) 
						+ " mipmap "+toString(mip)
						+ ". Probably, the GL driver refused to create the texture.");
                }
			}
		}
	}
	
	std::shared_ptr<GLPixelBuffer> GLTextureCore::getBuffer(UINT32 face, UINT32 mipmap)
	{
		THROW_IF_NOT_CORE_THREAD;

		if(face >= mProperties.getNumFaces())
			BS_EXCEPT(InvalidParametersException, "Face index out of range");

		if (mipmap > mProperties.getNumMipmaps())
			BS_EXCEPT(InvalidParametersException, "Mipmap index out of range");

		unsigned int idx = face * (mProperties.getNumMipmaps() + 1) + mipmap;
		assert(idx < mSurfaceList.size());
		return mSurfaceList[idx];
	}
}