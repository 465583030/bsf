//__________________________ Banshee Project - A modern game development toolkit _________________________________//
//_____________________________________ www.banshee-project.com __________________________________________________//
//________________________ Copyright (c) 2014 Marko Pintera. All rights reserved. ________________________________//
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

	RenderWindowPtr D3D11RenderWindowManager::createImpl(RENDER_WINDOW_DESC& desc, RenderWindowPtr parentWindow)
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
}