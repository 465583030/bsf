//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsRenderTarget.h"
#include "BsVideoModeInfo.h"
#include "BsVector2I.h"

namespace BansheeEngine
{
	/** @addtogroup RenderAPI
	 *  @{
	 */

	/** Structure that is used for initializing a render window. */
	struct BS_CORE_EXPORT RENDER_WINDOW_DESC
	{
		RENDER_WINDOW_DESC()
		: vsync(false), vsyncInterval(1), fullscreen(false), hidden(false), depthBuffer(true)
			, multisampleCount(0), multisampleHint(""), gamma(false), left(-1), top(-1)
			, title(""), border(WindowBorder::Normal), outerDimensions(false), enableDoubleClick(true)
			, toolWindow(false), modal(false), hideUntilSwap(false)
		{ }

		VideoMode videoMode; /**< A set of frame buffer options. */
		bool fullscreen; /**< Should the window be opened in fullscreen mode. */
		bool vsync; /**< Should the window wait for vertical sync before swapping buffers. */
		UINT32 vsyncInterval; /**< Determines how many vsync intervals occur per frame. FPS = refreshRate/interval. Usually 1 when vsync active. */
		bool hidden; /**< Should the window be hidden initially. */
		bool depthBuffer; /**< Should the window be created with a depth/stencil buffer. */
		UINT32 multisampleCount; /**< If higher than 1, texture containing multiple samples per pixel is created. */
		String multisampleHint; /**< Hint about what kind of multisampling to use. Render system specific. */
		bool gamma; /**< Should the written color pixels be gamma corrected before write. */
		INT32 left; /**< Window origin on X axis in pixels. -1 == screen center. Relative to monitor provided in videoMode. */
		INT32 top; /**< Window origin on Y axis in pixels. -1 == screen center. Relative to monitor provided in videoMode. */
		String title; /**< Title of the window. */
		WindowBorder border; /**< Type of border to create the window with. */
		bool outerDimensions; /**< Do our dimensions include space for things like title-bar and border. */
		bool enableDoubleClick; /**< Does window accept double-clicks. */
		bool toolWindow; /**< Tool windows have a different style than normal windows and can be created with no border or title bar. */
		bool modal; /**< When a modal window is open all other windows will be locked until modal window is closed. */
		bool hideUntilSwap; /** < Window will be created as hidden and only be shown when the first framebuffer swap happens. */

		NameValuePairList platformSpecific; /**< Platform-specific creation options. */
	};

	/**	Contains various properties that describe a render window. */
	class BS_CORE_EXPORT RenderWindowProperties : public RenderTargetProperties
	{
	public:
		RenderWindowProperties(const RENDER_WINDOW_DESC& desc);
		virtual ~RenderWindowProperties() { }

		/**	Gets the horizontal origin of the window in pixels. */
		INT32 getLeft() const { return mLeft; }

		/**	Gets the vertical origin of the window in pixels. */
		INT32 getTop() const { return mTop; }

		/**	Indicates whether the window currently has keyboard focus. */
		bool hasFocus() const { return mHasFocus; }

		/**	Returns true if window is running in fullscreen mode. */
		bool isFullScreen() const { return mIsFullScreen; }

		/**	Returns true if the window is modal (blocks interaction with any non-modal window until closed). */
		bool isModal() const { return mIsModal; }

		/**	Returns true if the window is hidden. */
		bool isHidden() const { return mHidden; }

		/**	Returns true if the window is maximized. */
		bool isMaximized() const { return mIsMaximized; }

	protected:
		friend class RenderWindowCore;
		friend class RenderWindow;

		bool mIsFullScreen = false;
		INT32 mLeft = 0;
		INT32 mTop = 0;
		bool mHasFocus = false;
		bool mHidden = false;
		bool mIsModal = false;
		bool mIsMaximized = false;
	};

	/** @cond INTERNAL */

	/**
	 * Provides access to internal render window implementation usable only from the core thread.
	 *
	 * @note	Core thread only.
	 */
	class BS_CORE_EXPORT RenderWindowCore : public RenderTargetCore
	{
	public:
		RenderWindowCore(const RENDER_WINDOW_DESC& desc, UINT32 windowId);
		virtual ~RenderWindowCore();

		/** 
		 * Switches the window to fullscreen mode. Child windows cannot go into fullscreen mode.
		 *
		 * @param[in]	width		Width of the window frame buffer in pixels.
		 * @param[in]	height		Height of the window frame buffer in pixels.
		 * @param[in]	refreshRate	Refresh rate of the window in Hertz.
		 * @param[in]	monitorIdx	Index of the monitor to go fullscreen on.
		 *
		 * @note	If the exact provided mode isn't available, closest one is used instead.
		 */
		virtual void setFullscreen(UINT32 width, UINT32 height, float refreshRate = 60.0f, UINT32 monitorIdx = 0) { }

		/**
		 * Switches the window to fullscreen mode. Child windows cannot go into fullscreen mode.
		 *
		 * @param[in]	videoMode	Mode retrieved from VideoModeInfo in RenderAPI.
		 */
		virtual void setFullscreen(const VideoMode& mode) { }

		/**
		 * Switches the window to windowed mode.
		 *
		 * @param[in]	Window width in pixels.
		 * @param[in]	Window height in pixels.
		 */
		virtual void setWindowed(UINT32 width, UINT32 height) { }

        /**	Hide or show the window. */
        virtual void setHidden(bool hidden);

		/**	Makes the render target active or inactive. (e.g. for a window, it will hide or restore the window). */
		virtual void setActive(bool state);

		/**	Minimizes the window to the taskbar. */
		virtual void minimize() { }

		/**	Maximizes the window over the entire current screen. */
		virtual void maximize() { }

		/**	Restores the window to original position and size if it is minimized or maximized. */
		virtual void restore() { }

        /**	Change the size of the window. */
        virtual void resize(UINT32 width, UINT32 height) = 0;

        /**	Reposition the window. */
        virtual void move(INT32 left, INT32 top) = 0;

		/**	Returns properties that describe the render window. */
		const RenderWindowProperties& getProperties() const;

		/**
		 * Called when window is moved or resized.
		 *
		 * @note	Core thread.
		 */
		virtual void _windowMovedOrResized();

		/**
		 * Called when window has received focus.
		 *
		 * @note	Core thread.
		 */
		virtual void _windowFocusReceived();

		/**
		 * Called when window has lost focus.
		 *
		 * @note	Core thread.
		 */
		virtual void _windowFocusLost();

		/**
		 * Called when window has been maximized.
		 *
		 * @note	Core thread.
		 */
		virtual void _notifyMaximized();

		/**
		 * Called when window has been minimized.
		 *
		 * @note	Core thread.
		 */
		virtual void _notifyMinimized();

		/**
		 * Called when window has been restored from minimized or maximized state.
		 *
		 * @note	Core thread.
		 */
		virtual void _notifyRestored();

	protected:
		friend class RenderWindow;
		friend class RenderWindowManager;
		friend class RenderWindowCoreManager;

		/**
		 * Returns window properties that are always kept in sync between core and sim threads.
		 *
		 * @note	Used for keeping up what are the most up to date settings.
		 */
		virtual RenderWindowProperties& getSyncedProperties() = 0;

		/** Updates window properties from the synced property data. */
		virtual void syncProperties() = 0;

		RENDER_WINDOW_DESC mDesc;
		SpinLock mLock;
		UINT32 mWindowId;
	};

	/** @endcond */

	/**
	 * Render target specialization that allows you to render into window frame buffer(s).
	 *
	 * @note	Sim thread only. Retrieve core implementation from getCore() for core thread only functionality.
	 */
    class BS_CORE_EXPORT RenderWindow : public RenderTarget
    {
    public:
		virtual ~RenderWindow() { }

		/** @copydoc RenderTarget::destroy */
		virtual void destroy() override;	

		/**	Converts screen position into window local position. */
		virtual Vector2I screenToWindowPos(const Vector2I& screenPos) const = 0;

		/**	Converts window local position to screen position. */
		virtual Vector2I windowToScreenPos(const Vector2I& windowPos) const = 0;

		/**	Resize the window to specified width and height in pixels. */
		void resize(CoreAccessor& accessor, UINT32 width, UINT32 height);

		/**	Move the window to specified screen coordinates. */
		void move(CoreAccessor& accessor, INT32 left, INT32 top);

		/**	Hide the window. (Does not destroy it, just hides it). */
		void hide(CoreAccessor& accessor);

		/**	Shows a previously hidden window. */
		void show(CoreAccessor& accessor);

		/** @copydoc RenderWindowCore::minimize */
		void minimize(CoreAccessor& accessor);

		/** @copydoc RenderWindowCore::maximize */
		void maximize(CoreAccessor& accessor);

		/** @copydoc RenderWindowCore::restore */
		void restore(CoreAccessor& accessor);

		/** @copydoc RenderWindowCore::setFullscreen(UINT32, UINT32, float, UINT32) */
		void setFullscreen(CoreAccessor& accessor, UINT32 width, UINT32 height, float refreshRate = 60.0f, UINT32 monitorIdx = 0);

		/** @copydoc RenderWindowCore::setFullscreen(const VideoMode&) */
		void setFullscreen(CoreAccessor& accessor, const VideoMode& mode);

		/** @copydoc RenderWindowCore::setWindowed */
		void setWindowed(CoreAccessor& accessor, UINT32 width, UINT32 height);

		/**	Retrieves a core implementation of a render window usable only from the core thread. */
		SPtr<RenderWindowCore> getCore() const;

		/**	Returns properties that describe the render window. */
		const RenderWindowProperties& getProperties() const;

		/**
		 * Creates a new render window using the specified options. Optionally makes the created window a child of another 
		 * window.
		 */
		static RenderWindowPtr create(RENDER_WINDOW_DESC& desc, RenderWindowPtr parentWindow = nullptr);

    protected:
		friend class RenderWindowManager;

		RenderWindow(const RENDER_WINDOW_DESC& desc, UINT32 windowId);

		/** Returns render window properties that may be edited. */
		RenderWindowProperties& getMutableProperties();

		/** @copydoc RenderTarget::createCore */
		SPtr<CoreObjectCore> createCore() const override;

		/**	Updates window properties from the synced property data. */
		virtual void syncProperties() = 0;

	protected:
		RENDER_WINDOW_DESC mDesc;
		UINT32 mWindowId;
    };

	/** @} */
}
