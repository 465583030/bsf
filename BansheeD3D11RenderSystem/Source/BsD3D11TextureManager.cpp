#include "BsD3D11TextureManager.h"
#include "BsD3D11Texture.h"
#include "BsD3D11RenderTexture.h"
#include "BsD3D11Mappings.h"
#include "BsD3D11RenderSystem.h"
#include "BsD3D11MultiRenderTexture.h"

namespace BansheeEngine
{
	D3D11TextureManager::D3D11TextureManager() 
		:TextureManager()
	{ }

	D3D11TextureManager::~D3D11TextureManager()
	{ }

	TexturePtr D3D11TextureManager::createTextureImpl()
	{
		D3D11Texture* tex = new (bs_alloc<D3D11Texture, PoolAlloc>()) D3D11Texture(); 

		return bs_core_ptr<D3D11Texture, PoolAlloc>(tex);
	}

	RenderTexturePtr D3D11TextureManager::createRenderTextureImpl()
	{
		D3D11RenderTexture* tex = new (bs_alloc<D3D11RenderTexture, PoolAlloc>()) D3D11RenderTexture();

		return bs_core_ptr<D3D11RenderTexture, PoolAlloc>(tex);
	}

	MultiRenderTexturePtr D3D11TextureManager::createMultiRenderTextureImpl()
	{
		D3D11MultiRenderTexture* tex = new (bs_alloc<D3D11MultiRenderTexture, PoolAlloc>()) D3D11MultiRenderTexture();

		return bs_core_ptr<D3D11MultiRenderTexture, PoolAlloc>(tex);
	}

	PixelFormat D3D11TextureManager::getNativeFormat(TextureType ttype, PixelFormat format, int usage, bool hwGamma)
	{
		// Basic filtering
		DXGI_FORMAT d3dPF = D3D11Mappings::getPF(D3D11Mappings::getClosestSupportedPF(format, hwGamma), hwGamma);

		return D3D11Mappings::getPF(d3dPF);
	}
}