//__________________________ Banshee Project - A modern game development toolkit _________________________________//
//_____________________________________ www.banshee-project.com __________________________________________________//
//________________________ Copyright (c) 2014 Marko Pintera. All rights reserved. ________________________________//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsTexture.h"
#include "BsRenderTarget.h"

namespace BansheeEngine
{    
	/**
	 * @brief	Structure that describes a render texture color and depth/stencil surfaces.
	 */
	struct BS_CORE_EXPORT RENDER_TEXTURE_DESC
	{
		RENDER_SURFACE_DESC colorSurface;
		RENDER_SURFACE_DESC depthStencilSurface;
	};

	/**
	 * @brief	Render target specialization that allows you to render into a texture you may
	 *			later bind in further render operations.
	 *
	 * @note	Thread safe, except where noted otherwise.
	 */
    class BS_CORE_EXPORT RenderTexture : public RenderTarget
    {
	public:
		virtual ~RenderTexture();

		/**
		 * @brief	Creates a new render texture with color and optionally depth/stencil surfaces.
		 *
		 * @param	textureType			Type of texture to render to.
		 * @param	width				Width of the render texture, in pixels.
		 * @param	height				Height of the render texture, in pixels.
		 * @param	format				Pixel format used by the texture color surface.
		 * @param	hwGamma				Should the written pixels be gamma corrected.
		 * @param	multisampleCount	If higher than 1, texture containing multiple samples per pixel is created.
		 * @param	multisampleHint		Hint about what kind of multisampling to use. Render system specific.
		 * @param	createDepth			Should a depth/stencil surface be created along with the color surface.
		 * @param	depthStencilFormat	Format used by the depth stencil surface, if one is created.
		 */
		static RenderTexturePtr create(TextureType textureType, UINT32 width, UINT32 height, 
			PixelFormat format = PF_R8G8B8A8, bool hwGamma = false, UINT32 multisampleCount = 0, 
			const String& multisampleHint = "", bool createDepth = true, PixelFormat depthStencilFormat = PF_D24S8);

		/**
		 * @copydoc RenderTarget::isWindow.
		 */
		bool isWindow() const { return false; }

		/**
		 * @copydoc RenderTarget::requiresTextureFlipping.
		 */
		bool requiresTextureFlipping() const { return false; }

		/**
		 * @brief	Returns a color surface texture you may bind as an input to an GPU program.
		 *
		 * @note	Be aware that you cannot bind a render texture for reading and writing at the same time.
		 */
		const HTexture& getBindableColorTexture() const { return mBindableColorTex; }

		/**
		* @brief	Returns a depth/stencil surface texture you may bind as an input to an GPU program.
		*
		* @note		Be aware that you cannot bind a render texture for reading and writing at the same time.
		*/
		const HTexture& getBindableDepthStencilTexture() const { return mBindableDepthStencilTex; }

	protected:
		friend class TextureManager;

		RenderTexture();

		/**
		 * @copydoc	RenderTarget::initialize
		 */
		void initialize(const RENDER_TEXTURE_DESC& desc);

		/**
		 * @copydoc RenderTarget::destroy_internal()
		 */
		virtual void destroy_internal();
	private:
		/**
		 * @brief	Throws an exception of the color and depth/stencil buffers aren't compatible.
		 */
		void throwIfBuffersDontMatch() const;

		/**
		 * @copydoc	RenderTarget::copyToMemory
		 */
		virtual void copyToMemory(PixelData &dst, FrameBuffer buffer = FB_AUTO);

	protected:
		TextureViewPtr mColorSurface;
		TextureViewPtr mDepthStencilSurface;

		HTexture mBindableColorTex;
		HTexture mBindableDepthStencilTex;
	};
}