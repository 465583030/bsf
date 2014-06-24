#pragma once

#include "BsD3D9Prerequisites.h"
#include "BsRenderWindow.h"
#include "BsD3D9Device.h"

namespace BansheeEngine 
{
	/**
	 * @brief	Render window implementation for Windows.
	 */
	class BS_D3D9_EXPORT D3D9RenderWindow : public RenderWindow
	{
	public:
		~D3D9RenderWindow();
		
		/**
		 * @copydoc RenderWindow::setFullscreen(UINT32, UINT32, float, UINT32)
		 */
		void setFullscreen(UINT32 width, UINT32 height, float refreshRate = 60.0f, UINT32 monitorIdx = 0);

		/**
		 * @copydoc RenderWindow::setFullscreen(const VideoMode&)
		 */
		void setFullscreen(const VideoMode& mode);

		/**
		* @copydoc RenderWindow::setWindowed
		*/
		void setWindowed(UINT32 width, UINT32 height);

		/**
		 * @copydoc RenderWindow::setHidden
		 */
		void setHidden(bool hidden);

		/**
		 * @copydoc RenderWindow::isActive
		 */
		bool isActive() const;

		/**
		 * @copydoc RenderWindow::isVisible
		 */
		bool isVisible() const;

		/**
		 * @copydoc RenderWindow::isClosed
		 */
		bool isClosed() const { return mClosed; }

		/**
		 * @copydoc RenderWindow::isVSync
		 */
		bool isVSync() const { return mVSync; }

		/**
		 * @copydoc RenderWindow::move
		 */
		void move(INT32 left, INT32 top);

		/**
		 * @copydoc RenderWindow::resize
		 */
		void resize(UINT32 width, UINT32 height);

		/**
		 * @copydoc RenderWindow::getCustomAttribute
		 */
		void getCustomAttribute(const String& name, void* pData) const;

		/**
		 * @copydoc RenderWindow::copyContentsToMemory
		 */
		void copyToMemory(const PixelData &dst, FrameBuffer buffer);

		/**
		 * @copydoc RenderWindow::requiresTextureFlipping
		 */
		bool requiresTextureFlipping() const { return false; }

		/**
		 * @copydoc RenderWindow::swapBuffers
		 */
		void swapBuffers();

		/**
		 * @copydoc RenderWindow::screenToWindowPos
		 */
		Vector2I screenToWindowPos(const Vector2I& screenPos) const;

		/**
		 * @copydoc RenderWindow::windowToScreenPos
		 */
		Vector2I windowToScreenPos(const Vector2I& windowPos) const;

		/**
		 * @copydoc RenderWindow::_windowMovedOrResized
		 */
		void _windowMovedOrResized();

		/**
		 * @brief	Gets internal Win32 window handle.
		 */
		HWND _getWindowHandle() const { return mHWnd; }

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
		friend class D3D9RenderWindowManager;

		D3D9RenderWindow(const RENDER_WINDOW_DESC& desc, HINSTANCE instance);

		/**
		 * @brief	Updates window coordinates and size from actual values provided by Windows.
		 */
		void updateWindowRect();

		/**
		 * @copydoc RenderWindow::initialize_internal().
		 */
		void initialize_internal();

		/**
		 * @copydoc RenderWindow::destroy_internal().
		 */
		void destroy_internal();

		/**
		 * @brief	Calculates window size based on provided client area size and currently set window style. 
		 */	
		void getAdjustedWindowSize(UINT32 clientWidth, UINT32 clientHeight,
			DWORD style, UINT32* winWidth, UINT32* winHeight);

	protected:
		HINSTANCE mInstance;
		D3D9Device* mDevice;
		bool mDeviceValid;
		HWND mHWnd;
		bool mIsExternal;
		bool mClosed;		
		D3DMULTISAMPLE_TYPE	mMultisampleType;
		DWORD mMultisampleQuality;
		UINT mDisplayFrequency;
		bool mVSync;
		unsigned int mVSyncInterval;		
		DWORD mStyle;	
		DWORD mWindowedStyle;
		DWORD mWindowedStyleEx;
		bool mIsDepthBuffered;
		bool mIsChild;
	};
}