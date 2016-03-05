//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsD3D11Prerequisites.h"
#include "BsTexture.h"

namespace BansheeEngine
{
	/**	DirectX 11 implementation of a texture. */
	class D3D11TextureCore : public TextureCore
	{
	public:
		~D3D11TextureCore();

		/**	Returns internal DX11 texture resource object. */
		ID3D11Resource* getDX11Resource() const { return mTex; }

		/**	Returns shader resource view associated with the texture. */
		ID3D11ShaderResourceView* getSRV() const { return mShaderResourceView; }

		/** Returns DXGI pixel format that was used to create the texture. */
		DXGI_FORMAT getDXGIFormat() const { return mDXGIFormat; }

		/** Returns DXGI pixel used for reading the texture as a shader resource or writing as a render target. */
		DXGI_FORMAT getColorFormat() const { return mDXGIColorFormat; }

		/** Returns DXGI pixel used for writing to a depth stencil texture. */
		DXGI_FORMAT getDepthStencilFormat() const { return mDXGIDepthStencilFormat; }

	protected:
		friend class D3D11TextureCoreManager;

		D3D11TextureCore(TextureType textureType, UINT32 width, UINT32 height, UINT32 depth, UINT32 numMipmaps,
			PixelFormat format, int usage, bool hwGamma, UINT32 multisampleCount, const PixelDataPtr& initialData);

		/** @copydoc CoreObjectCore::initialize() */
		void initialize() override;

		/** @copydoc TextureCore::lockImpl */
		PixelData lockImpl(GpuLockOptions options, UINT32 mipLevel = 0, UINT32 face = 0) override;

		/** @copydoc TextureCore::unlockImpl */
		void unlockImpl() override;

		/** @copydoc TextureCore::copyImpl */
		void copyImpl(UINT32 srcFace, UINT32 srcMipLevel, UINT32 destFace, UINT32 destMipLevel, const SPtr<TextureCore>& target) override;

		/** @copydoc TextureCore::readData */
		void readData(PixelData& dest, UINT32 mipLevel = 0, UINT32 face = 0) override;

		/** @copydoc TextureCore::writeData */
		void writeData(const PixelData& src, UINT32 mipLevel = 0, UINT32 face = 0, bool discardWholeBuffer = false) override;

		/**	Creates a blank DX11 1D texture object. */
		void create1DTex();

		/**	Creates a blank DX11 2D texture object. */
		void create2DTex();

		/**	Creates a blank DX11 3D texture object. */
		void create3DTex();

		/**
		 * Creates a staging buffer that is used as a temporary buffer for read operations on textures that do not support
		 * direct reading.
		 */
		void createStagingBuffer();

		/**
		 * Maps the specified texture surface for reading/writing. 
		 *
		 * @param[in]	res			Texture resource to map.
		 * @param[in]	flags		Mapping flags that let the API know what are we planning to do with mapped memory.
		 * @param[in]	mipLevel	Mip level to map (0 being the base level).
		 * @param[in]	face		Texture face to map, in case texture has more than one.
		 * @param[out]	rowPitch	Output size of a single row in bytes.
		 * @param[out]	slicePitch	Output size of a single slice in bytes (relevant only for 3D textures).
		 * @return					Pointer to the mapped area of memory.
		 *
		 * @note	
		 * Non-staging textures must be dynamic in order to be mapped directly and only for writing. No restrictions are
		 * made on staging textures.
		 */
		void* map(ID3D11Resource* res, D3D11_MAP flags, UINT32 mipLevel, UINT32 face, UINT32& rowPitch, UINT32& slicePitch);

		/**	Unmaps a previously mapped texture. */
		void unmap(ID3D11Resource* res);

		/**
		 * Copies texture data into a staging buffer and maps the staging buffer. Will create a staging buffer if one 
		 * doesn't already exist (potentially wasting a lot of memory).
		 *
		 * @param[in]	flags		Mapping flags that let the API know what are we planning to do with mapped memory.
		 * @param[in]	mipLevel	Mip level to map (0 being the base level).
		 * @param[in]	face		Texture face to map, in case texture has more than one.
		 * @param[out]	rowPitch	Output size of a single row in bytes.
		 * @param[out]	slicePitch	Output size of a single slice in bytes (relevant only for 3D textures).
		 * @return					Pointer to the mapped area of memory.
		 */
		void* mapstagingbuffer(D3D11_MAP flags, UINT32 mipLevel, UINT32 face, UINT32& rowPitch, UINT32& slicePitch);

		/**	Unmaps a previously mapped staging buffer. */
		void unmapstagingbuffer();
		
		/**
		 * Maps a static buffer, for writing only. Returned pointer points to temporary CPU memory that will be copied to
		 * the mapped resource on "unmap" call.
		 *
		 * @param[in]	flags		Mapping flags that let the API know what are we planning to do with mapped memory.
		 * @param[in]	mipLevel	Mip level to map (0 being the base level).
		 * @param[in]	face		Texture face to map, in case texture has more than one.
		 * @param[out]	rowPitch	Output size of a single row in bytes.
		 * @param[out]	slicePitch	Output size of a single slice in bytes (relevant only for 3D textures).
		 * @return					Pointer to the mapped area of memory.
		 */
		void* mapstaticbuffer(PixelData lock, UINT32 mipLevel, UINT32 slice);

		/**	Unmaps a previously mapped static buffer and flushes its data to the actual GPU buffer. */
		void unmapstaticbuffer();

		/**	Creates an empty and uninitialized texture view object. */
		TextureViewPtr createView(const SPtr<TextureCore>& texture, const TEXTURE_VIEW_DESC& desc) override;

	protected:
		ID3D11Texture1D* m1DTex;
		ID3D11Texture2D* m2DTex;
		ID3D11Texture3D* m3DTex;
		ID3D11Resource* mTex;

		ID3D11ShaderResourceView* mShaderResourceView;
		D3D11_SHADER_RESOURCE_VIEW_DESC mSRVDesc;
		D3D11_SRV_DIMENSION mDimension;
		DXGI_FORMAT mDXGIFormat;
		DXGI_FORMAT mDXGIColorFormat;
		DXGI_FORMAT mDXGIDepthStencilFormat;

		ID3D11Resource* mStagingBuffer;
		PixelData* mStaticBuffer;
		UINT32 mLockedSubresourceIdx;
		bool mLockedForReading;
	};
}