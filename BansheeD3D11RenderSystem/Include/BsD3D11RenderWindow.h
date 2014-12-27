#pragma once

#include "BsD3D11Prerequisites.h"
#include "BsRenderWindow.h"

namespace BansheeEngine
{
	class D3D11RenderWindow;

	/**
	 * @brief	Contains various properties that describe a render window.
	 */
	class BS_D3D11_EXPORT D3D11RenderWindowProperties : public RenderWindowProperties
	{
	public:
		D3D11RenderWindowProperties(const RENDER_WINDOW_DESC& desc);
		virtual ~D3D11RenderWindowProperties() { }

	private:
		friend class D3D11RenderWindowCore;
		friend class D3D11RenderWindow;
	};

	/**
	 * @brief	Render window implementation for Windows and DirectX 11.
	 *
	 * @note	Core thread only.
	 */
	class BS_D3D11_EXPORT D3D11RenderWindowCore : public RenderWindowCore
	{
	public:
		/**
		 * @copydoc	RenderWindowCore::RenderWindowCore
		 */
		D3D11RenderWindowCore(const RENDER_WINDOW_DESC& desc,
			D3D11Device& device, IDXGIFactory* DXGIFactory);

		~D3D11RenderWindowCore();

		/**
		 * @copydoc RenderWindowCore::move
		 */
		void move(INT32 left, INT32 top);

		/**
		 * @copydoc RenderWindowCore::resize
		 */
		void resize(UINT32 width, UINT32 height);

		/**
		 * @copydoc RenderWindowCore::setHidden
		 */
		void setHidden(bool hidden);

		/**
		 * @copydoc RenderWindowCore::setActive
		 */
		void setActive(bool state);

		/**
		 * @copydoc RenderWindowCore::setFullscreen(UINT32, UINT32, float, UINT32)
		 */
		void setFullscreen(UINT32 width, UINT32 height, float refreshRate = 60.0f, UINT32 monitorIdx = 0);

		/**
		 * @copydoc RenderWindowCore::setFullscreen(const VideoMode&)
		 */
		void setFullscreen(const VideoMode& mode);

		/**
		* @copydoc RenderWindowCore::setWindowed
		*/
		void setWindowed(UINT32 width, UINT32 height);

		/**
		 * @copydoc RenderWindowCore::copyContentsToMemory
		 */
		void copyToMemory(PixelData &dst, FrameBuffer buffer);

		/**
		 * @copydoc RenderWindowCore::swapBuffers
		 */
		void swapBuffers();

		/**
		 * @copydoc RenderWindowCore::getCustomAttribute
		 */
		void getCustomAttribute(const String& name, void* pData) const;

		/**
		 * @copydoc	RenderWindowCore::_windowMovedOrResized
		 */
		void _windowMovedOrResized();

		/**
		 * @brief	Returns presentation parameters used for creating the window swap chain.
		 */
		DXGI_SWAP_CHAIN_DESC* _getPresentationParameters() { return &mSwapChainDesc; }

		/**
		 * @brief	Returns internal window handle.
		 */
		HWND _getWindowHandle() const;

	protected:
		friend class D3D11RenderWindow;

		/**
		 * @copydoc	CoreObjectCore::initialize
		 */
		virtual void initialize();

		/**
		 * @brief	Creates internal resources dependent on window size.
		 */
		void createSizeDependedD3DResources();

		/**
		 * @brief	Destroys internal resources dependent on window size.
		 */
		void destroySizeDependedD3DResources();

		/**
		 * @brief	Queries the current DXGI device. Make sure to release the returned object when done with it.
		 */
		IDXGIDevice* queryDxgiDevice(); 

		/**
		 * @brief	Creates a swap chain for the window.
		 */
		void createSwapChain();

		/**
		 * @brief	Resizes all buffers attached to the swap chain to the specified size.
		 */
		void resizeSwapChainBuffers(UINT32 width, UINT32 height);

		/**
		 * @copydoc	RenderWindowCore::getProperties
		 */
		const RenderTargetProperties& getPropertiesInternal() const { return mProperties; }

		/**
		 * @copydoc	RenderWindowCore::getSyncData
		 */
		UINT32 getSyncData(UINT8* buffer);

	protected:
		D3D11Device& mDevice;
		IDXGIFactory* mDXGIFactory;
		bool mIsExternal;
		bool mSizing;
		bool mIsChild;

		DXGI_SAMPLE_DESC mMultisampleType;
		UINT32 mRefreshRateNumerator;
		UINT32 mRefreshRateDenominator;

		ID3D11Texture2D* mBackBuffer;
		ID3D11RenderTargetView*	mRenderTargetView;
		TextureViewPtr mDepthStencilView;
		SPtr<TextureCore> mDepthStencilBuffer;

		IDXGISwapChain*	mSwapChain;
		DXGI_SWAP_CHAIN_DESC mSwapChainDesc;
		HWND mHWnd;

		D3D11RenderWindowProperties mProperties;
	};

	/**
	 * @brief	Render window implementation for Windows and DirectX 11.
	 *
	 * @note	Sim thread only.
	 */
	class BS_D3D11_EXPORT D3D11RenderWindow : public RenderWindow
	{
	public:
		~D3D11RenderWindow() { }

		/**
		 * @copydoc RenderWindow::screenToWindowPos
		 */
		void getCustomAttribute(const String& name, void* pData) const;

		/**
		 * @copydoc RenderWindow::screenToWindowPos
		 */
		Vector2I screenToWindowPos(const Vector2I& screenPos) const;

		/**
		 * @copydoc RenderWindow::windowToScreenPos
		 */
		Vector2I windowToScreenPos(const Vector2I& windowPos) const;

		/**
		 * @copydoc	RenderWindow::getCore
		 */
		SPtr<D3D11RenderWindowCore> getCore() const;

	protected:
		friend class D3D11RenderWindowManager;
		friend class D3D11RenderWindowCore;

		D3D11RenderWindow(const RENDER_WINDOW_DESC& desc, D3D11Device& device, IDXGIFactory* DXGIFactory);

		/**
		 * @copydoc	RenderWindowCore::getProperties
		 */
		const RenderTargetProperties& getPropertiesInternal() const { return mProperties; }

		/**
		 * @copydoc	RenderWindow::setSyncData
		 */
		void setSyncData(UINT8* buffer, UINT32 size);

		/**
		 * @brief	Retrieves internal window handle.
		 */
		HWND getHWnd() const;

	private:
		D3D11Device& mDevice;
		IDXGIFactory* mDXGIFactory;
		D3D11RenderWindowProperties mProperties;
	};
}