//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsD3D9Prerequisites.h"
#include "BsMultiRenderTexture.h"

namespace BansheeEngine
{
	/** @addtogroup D3D9
	 *  @{
	 */

	class D3D9MultiRenderTexture;

	/**
	 * DirectX 9 implementation of a render texture with multiple color surfaces.
	 *
	 * @note	Core thread only.
	 */
	class BS_D3D9_EXPORT D3D9MultiRenderTextureCore : public MultiRenderTextureCore
	{
	public:
		D3D9MultiRenderTextureCore(const MULTI_RENDER_TEXTURE_CORE_DESC& desc);
		virtual ~D3D9MultiRenderTextureCore();
		
		/** @copydoc MultiRenderTextureCore::getCustomAttribute */
		void getCustomAttribute(const String& name, void* pData) const override;

	protected:
		friend class D3D9MultiRenderTexture;

		/** @copydoc CoreObjectCore::initialize */
		void initialize() override;

		/** @copydoc MultiRenderTextureCore::getProperties */
		const RenderTargetProperties& getPropertiesInternal() const override { return mProperties; }

		Vector<IDirect3DSurface9*> mDX9ColorSurfaces;
		IDirect3DSurface9* mDX9DepthStencilSurface;

		MultiRenderTextureProperties mProperties;
	};

	/**
	 * DirectX 9 implementation of a render texture with multiple color surfaces.
	 *
	 * @note	Sim thread only.
	 */
	class BS_D3D9_EXPORT D3D9MultiRenderTexture : public MultiRenderTexture
	{
	public:
		virtual ~D3D9MultiRenderTexture() { }

	protected:
		friend class D3D9TextureManager;

		D3D9MultiRenderTexture(const MULTI_RENDER_TEXTURE_DESC& desc);

		/** @copydoc MultiRenderTexture::getProperties */
		const RenderTargetProperties& getPropertiesInternal() const override { return mProperties; }

		MultiRenderTextureProperties mProperties;
	};

	/** @} */
}