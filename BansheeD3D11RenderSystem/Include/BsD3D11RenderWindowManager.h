#pragma once

#include "BsD3D11Prerequisites.h"
#include "BsRenderWindowManager.h"

namespace BansheeEngine
{
	/**
	 * @copydoc	RenderWindowManager
	 */
	class BS_D3D11_EXPORT D3D11RenderWindowManager : public RenderWindowManager
	{
	public:
		D3D11RenderWindowManager(D3D11RenderSystem* renderSystem);

	protected:
		/**
		 * @copydoc RenderWindowManager::createImpl
		 */
		RenderWindowPtr createImpl(RENDER_WINDOW_DESC& desc, RenderWindowPtr parentWindow);

	private:
		D3D11RenderSystem* mRenderSystem;
	};
}