#include "BsD3D11RenderWindowManager.h"
#include "BsD3D11RenderSystem.h"
#include "BsD3D11RenderWindow.h"
#include "BsAsyncOp.h"

namespace BansheeEngine
{
	D3D11RenderWindowManager::D3D11RenderWindowManager(D3D11RenderSystem* renderSystem)
		:mRenderSystem(renderSystem)
	{
		assert(mRenderSystem != nullptr);
	}

	RenderWindowPtr D3D11RenderWindowManager::createImpl(RENDER_WINDOW_DESC& desc, const RenderWindowPtr& parentWindow)
	{
		RenderSystem* rs = RenderSystem::instancePtr();
		D3D11RenderSystem* d3d11rs = static_cast<D3D11RenderSystem*>(rs);

		if(parentWindow != nullptr)
		{
			HWND hWnd;
			parentWindow->getCustomAttribute("WINDOW", &hWnd);
			desc.platformSpecific["parentWindowHandle"] = toString((UINT64)hWnd);
		}

		// Create the window
		D3D11RenderWindow* renderWindow = new (bs_alloc<D3D11RenderWindow, PoolAlloc>()) D3D11RenderWindow(desc, d3d11rs->getPrimaryDevice(), d3d11rs->getDXGIFactory());
		return bs_core_ptr<D3D11RenderWindow, PoolAlloc>(renderWindow);
	}

	D3D11RenderWindowCoreManager::D3D11RenderWindowCoreManager(D3D11RenderSystem* renderSystem)
		:mRenderSystem(renderSystem)
	{
		assert(mRenderSystem != nullptr);
	}

	SPtr<RenderWindowCore> D3D11RenderWindowCoreManager::createInternal(RENDER_WINDOW_DESC& desc)
	{
		RenderSystem* rs = RenderSystem::instancePtr();
		D3D11RenderSystem* d3d11rs = static_cast<D3D11RenderSystem*>(rs);

		// Create the window
		D3D11RenderWindowCore* renderWindow = new (bs_alloc<D3D11RenderWindowCore, GenAlloc>()) D3D11RenderWindowCore(desc, d3d11rs->getPrimaryDevice(), d3d11rs->getDXGIFactory());
		windowCreated(renderWindow);

		return bs_shared_ptr<D3D11RenderWindowCore, GenAlloc>(renderWindow);
	}
}