//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGLTextureManager.h"
#include "BsRenderAPI.h"
#include "BsGLRenderTexture.h"
#include "BsGLMultiRenderTexture.h"
#include "BsGLPixelFormat.h"

namespace BansheeEngine
{
    GLTextureManager::GLTextureManager(GLSupport& support)
        :TextureManager(), mGLSupport(support)
    {

    }

    GLTextureManager::~GLTextureManager()
    {

    }

	SPtr<RenderTexture> GLTextureManager::createRenderTextureImpl(const RENDER_TEXTURE_DESC& desc)
	{
		GLRenderTexture* tex = new (bs_alloc<GLRenderTexture>()) GLRenderTexture(desc);

		return bs_core_ptr<GLRenderTexture>(tex);
	}

	SPtr<MultiRenderTexture> GLTextureManager::createMultiRenderTextureImpl(const MULTI_RENDER_TEXTURE_DESC& desc)
	{
		GLMultiRenderTexture* tex = new (bs_alloc<GLMultiRenderTexture>()) GLMultiRenderTexture(desc);

		return bs_core_ptr<GLMultiRenderTexture>(tex);
	}

	PixelFormat GLTextureManager::getNativeFormat(TextureType ttype, PixelFormat format, int usage, bool hwGamma)
	{
		// Adjust requested parameters to capabilities
        const RenderAPICapabilities *caps = RenderAPICore::instancePtr()->getCapabilities();

		// Check compressed texture support
		// If a compressed format not supported, revert to PF_A8R8G8B8
		if(PixelUtil::isCompressed(format) && !caps->hasCapability(RSC_TEXTURE_COMPRESSION_DXT))
		{
			return PF_A8R8G8B8;
		}

		// If floating point textures not supported, revert to PF_A8R8G8B8
		if(PixelUtil::isFloatingPoint(format) && !caps->hasCapability(RSC_TEXTURE_FLOAT))
		{
			return PF_A8R8G8B8;
		}
        
        // Check if this is a valid rendertarget format
		if( usage & TU_RENDERTARGET )
        {
            /// Get closest supported alternative
            /// If mFormat is supported it's returned
            return GLRTTManager::instance().getSupportedAlternative(format);
        }

		return GLPixelUtil::getClosestValidFormat(format);
	}

	GLTextureCoreManager::GLTextureCoreManager(GLSupport& support)
		:mGLSupport(support)
	{ }

	SPtr<TextureCore> GLTextureCoreManager::createTextureInternal(TextureType texType, UINT32 width, UINT32 height, UINT32 depth,
		int numMips, PixelFormat format, int usage, bool hwGammaCorrection, UINT32 multisampleCount, UINT32 numArraySlices, const SPtr<PixelData>& initialData)
	{
		GLTextureCore* tex = new (bs_alloc<GLTextureCore>()) GLTextureCore(mGLSupport, texType,
			width, height, depth, numMips, format, usage, hwGammaCorrection, multisampleCount, numArraySlices, initialData);

		SPtr<GLTextureCore> texPtr = bs_shared_ptr<GLTextureCore>(tex);
		texPtr->_setThisPtr(texPtr);

		return texPtr;
	}

	SPtr<RenderTextureCore> GLTextureCoreManager::createRenderTextureInternal(const RENDER_TEXTURE_CORE_DESC& desc)
	{
		SPtr<GLRenderTextureCore> texPtr = bs_shared_ptr_new<GLRenderTextureCore>(desc);
		texPtr->_setThisPtr(texPtr);

		return texPtr;
	}

	SPtr<MultiRenderTextureCore> GLTextureCoreManager::createMultiRenderTextureInternal(const MULTI_RENDER_TEXTURE_CORE_DESC& desc)
	{
		SPtr<GLMultiRenderTextureCore> texPtr = bs_shared_ptr_new<GLMultiRenderTextureCore>(desc);
		texPtr->_setThisPtr(texPtr);

		return texPtr;
	}
}
