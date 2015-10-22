#pragma once

#include "BsD3D9Prerequisites.h"
#include "BsRenderWindow.h"
#include "BsD3D9Device.h"

namespace BansheeEngine 
{
	class D3D9RenderWindow;

	/**
	 * @brief	Contains various properties that describe a render window.
	 */
	class BS_D3D9_EXPORT D3D9RenderWindowProperties : public RenderWindowProperties
	{
	public:
		D3D9RenderWindowProperties(const RENDER_WINDOW_DESC& desc);
		virtual ~D3D9RenderWindowProperties() { }

	private:
		friend class D3D9RenderWindowCore;
		friend class D3D9RenderWindow;
	};

	/**
	 * @brief	Render window implementation for Windows.
	 *
	 * @note	Core thread only.
	 */
	class BS_D3D9_EXPORT D3D9RenderWindowCore : public RenderWindowCore
	{
	public:
		D3D9RenderWindowCore(const RENDER_WINDOW_DESC& desc, UINT32 windowId, HINSTANCE instance);
		~D3D9RenderWindowCore();
		
		/**
		 * @copydoc RenderWindowCore::setFullscreen(UINT32, UINT32, float, UINT32)
		 */
		void setFullscreen(UINT32 width, UINT32 height, float refreshRate = 60.0f, UINT32 monitorIdx = 0) override;

		/**
		 * @copydoc RenderWindowCore::setFullscreen(const VideoMode&)
		 */
		void setFullscreen(const VideoMode& mode) override;

		/**
		* @copydoc RenderWindowCore::setWindowed
		*/
		void setWindowed(UINT32 width, UINT32 height) override;

		/**
		 * @copydoc RenderWindowCore::setActive
		 */
		virtual void setActive(bool state) override;

		/**
		 * @copydoc RenderWindowCore::setHidden
		 */
		void setHidden(bool hidden) override;

		/**
		 * @copydoc	RenderWindowCore::minimize
		 */
		void minimize() override;

		/**
		 * @copydoc	RenderWindowCore::maximize
		 */
		void maximize() override;

		/**
		 * @copydoc	RenderWindowCore::restore
		 */
		void restore() override;

		/**
		 * @copydoc RenderWindowCore::move
		 */
		void move(INT32 left, INT32 top) override;

		/**
		 * @copydoc RenderWindowCore::resize
		 */
		void resize(UINT32 width, UINT32 height) override;

		/**
		 * @copydoc RenderWindowCore::getCustomAttribute
		 */
		void getCustomAttribute(const String& name, void* pData) const override;

		/**
		 * @copydoc RenderWindowCore::copyContentsToMemory
		 */
		void copyToMemory(PixelData &dst, FrameBuffer buffer);

		/**
		 * @copydoc RenderWindowCore::swapBuffers
		 */
		void swapBuffers() override;

		/**
		 * @copydoc RenderWindowCore::_windowMovedOrResized
		 */
		void _windowMovedOrResized() override;

		/**
		 * @brief	Gets internal Win32 window handle.
		 */
		HWND _getWindowHandle() const;

		/**
		 * @brief	Gets the DirectX 9 device object that manages this window.
		 */
		IDirect3DDevice9* _getD3D9Device() const;

		/**
		 * @brief	Gets the device that manages this window.
		 */
		D3D9Device* _getDevice() const;

		/**
		 * @brief	Sets the device that manages this window.
		 */
		void _setDevice(D3D9Device* device);

		/**
		 * @brief	Build the presentation parameters used with this window.
		 */
		void _buildPresentParameters(D3DPRESENT_PARAMETERS* presentParams) const;

		/**
		 * @brief	Accessor for render surface.
		 */
		IDirect3DSurface9* _getRenderSurface() const;
	
		/**
		 * @brief	Returns true if this window use depth buffer.
		 */
		bool _isDepthBuffered() const;

		/**
		 * @brief	Validate the device for this window.
		 */
		bool _validateDevice();
		
	protected:
		friend class D3D9RenderWindow;

		/**
		 * @copydoc	CoreObjectCore::initialize
		 */
		virtual void initialize() override;

		/**
		 * @copydoc	RenderWindowCore::getProperties
		 */
		const RenderTargetProperties& getPropertiesInternal() const override { return mProperties; }

		/**
		 * @copydoc	RenderWindowCore::getSyncedProperties
		 */
		RenderWindowProperties& getSyncedProperties() override { return mSyncedProperties; }

		/**
		 * @copydoc	RenderWindowCore::syncProperties
		 */
		void syncProperties() override;

	protected:
		Win32Window* mWindow;
		HINSTANCE mInstance;
		D3D9Device* mDevice;
		bool mDeviceValid;
		D3DMULTISAMPLE_TYPE	mMultisampleType;
		DWORD mMultisampleQuality;
		UINT mDisplayFrequency;
		unsigned int mVSyncInterval;
		bool mIsDepthBuffered;
		bool mIsChild;
		bool mShowOnSwap;
		
		D3D9RenderWindowProperties mProperties;
		D3D9RenderWindowProperties mSyncedProperties;
	};

	/**
	 * @brief	Render window implementation for Windows.
	 *
	 * @note	Sim thread only.
	 */
	class BS_D3D9_EXPORT D3D9RenderWindow : public RenderWindow
	{
	public:
		~D3D9RenderWindow() { }

		/**
		 * @copydoc RenderWindow::screenToWindowPos
		 */
		void getCustomAttribute(const String& name, void* pData) const override;

		/**
		 * @copydoc RenderWindow::screenToWindowPos
		 */
		Vector2I screenToWindowPos(const Vector2I& screenPos) const override;

		/**
		 * @copydoc RenderWindow::windowToScreenPos
		 */
		Vector2I windowToScreenPos(const Vector2I& windowPos) const override;

		/**
		 * @copydoc	RenderWindow::getCore
		 */
		SPtr<D3D9RenderWindowCore> getCore() const;

	protected:
		friend class D3D9RenderWindowManager;
		friend class D3D9RenderWindowCore;

		D3D9RenderWindow(const RENDER_WINDOW_DESC& desc, UINT32 windowId, HINSTANCE instance);

		/**
		 * @copydoc	RenderWindowCore::getProperties
		 */
		const RenderTargetProperties& getPropertiesInternal() const override { return mProperties; }

		/**
		 * @copydoc	RenderWindow::syncProperties
		 */
		void syncProperties() override;

		/**
		 * @brief	Retrieves internal window handle.
		 */
		HWND getHWnd() const;

	private:
		HINSTANCE mInstance;
		D3D9RenderWindowProperties mProperties;
	};
}