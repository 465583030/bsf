#pragma once

#include "BsD3D11Prerequisites.h"
#include "BsTexture.h"
#include "BsRenderTexture.h"

namespace BansheeEngine
{
	class D3D11RenderTexture;

	/**
	 * @brief	DirectX 11 implementation of a render texture.
	 *
	 * @note	Core thread only.
	 */
	class D3D11RenderTextureCore : public RenderTextureCore
	{
	public:
		D3D11RenderTextureCore(const RENDER_TEXTURE_DESC& desc);
		virtual ~D3D11RenderTextureCore() { }

		/**
		 * @copydoc	RenderTextureCore::getCustomAttribute
		 */
		void getCustomAttribute(const String& name, void* pData) const;

	protected:
		/**
		 * @copydoc	RenderTextureCore::getProperties
		 */
		const RenderTargetProperties& getPropertiesInternal() const { return mProperties; }

		RenderTextureProperties mProperties;
	};

	/**
	 * @brief	DirectX 11 implementation of a render texture.
	 *
	 * @note	Sim thread only.
	 */
	class D3D11RenderTexture : public RenderTexture
	{
	public:
		virtual ~D3D11RenderTexture() { }

	protected:
		friend class D3D11TextureManager;

		D3D11RenderTexture(const RENDER_TEXTURE_DESC& desc);

		/**
		 * @copydoc	RenderTexture::getProperties
		 */
		const RenderTargetProperties& getPropertiesInternal() const { return mProperties; }

		RenderTextureProperties mProperties;
	};
}