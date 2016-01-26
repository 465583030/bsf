#pragma once

#include "BsCorePrerequisites.h"

#include "BsRenderTarget.h"
#include "BsVideoModeInfo.h"
#include "BsVector2I.h"

namespace BansheeEngine
{
	/**
	 * @brief	Enum that defines possible window border styles.
	 */
	enum class WindowBorder
	{
		Normal,
		None,
		Fixed
	};

	/**
	 * @brief	Structure that is used for initializing a render window.
	 */
	struct BS_CORE_EXPORT RENDER_WINDOW_DESC
	{
		RENDER_WINDOW_DESC()
		: vsync(false), vsyncInterval(1), fullscreen(false), hidden(false), depthBuffer(true)
			, multisampleCount(0), multisampleHint(""), gamma(false), left(-1), top(-1)
			, title(""), border(WindowBorder::Normal), outerDimensions(false), enableDoubleClick(true)
			, toolWindow(false), modal(false)
		{ }

		VideoMode videoMode; /**< A set of frame buffer options. */
		bool fullscreen; /**< Should the window be opened in fullscreen mode. */
		bool vsync; /**< Should the window wait for vertical sync before swapping buffers. */
		UINT32 vsyncInterval; /**< Determines how many vsync intervals occur per frame. FPS = refreshRate/interval. Usually 1 when vsync active. */
		bool hidden; /**< Should the window be hidden. */
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

		NameValuePairList platformSpecific; /**< Platform-specific creation options. */
	};

	/**
	 * @brief	Render target specialization that allows you to render into window
	 *			frame buffer(s).
	 *
	 * @note	Thread safe, except where noted otherwise.
	 */
    class BS_CORE_EXPORT RenderWindow : public RenderTarget
    {
    public:
		virtual ~RenderWindow();

		/** 
		 * @brief	Switches the window to fullscreen mode. Child windows cannot go into fullscreen mode.
		 *
		 * @param	width		Width of the window frame buffer in pixels.
		 * @param	height		Height of the window frame buffer in pixels.
		 * @param	refreshRate	Refresh rate of the window in Hertz.
		 * @param	monitorIdx	Index of the monitor to go fullscreen on.
		 *
		 * @note	Core thread.
		 *			If the exact provided mode isn't available, closest one is used instead.
		 */
		virtual void setFullscreen(UINT32 width, UINT32 height, float refreshRate = 60.0f, UINT32 monitorIdx = 0) { }

		/**
		* @brief	Switches the window to fullscreen mode. Child windows cannot go into fullscreen mode.
		*
		* @param	videoMode	Mode retrieved from VideoModeInfo in RenderSystem.
		*
		* @note		Core thread.
		*/
		virtual void setFullscreen(const VideoMode& mode) { }

		/**
		 * @brief	Switches the window to windowed mode.
		 *
		 * @param	Window width in pixels.
		 * @param	Window height in pixels.
		 *
		 * @note	Core thread.
		 */
		virtual void setWindowed(UINT32 width, UINT32 height) { }

        /**
         * @brief	Hide or show the window.
		 *
		 * @note	Core thread.
         */
        virtual void setHidden(bool hidden);

        /**
         * @brief	Change the size of the window.
		 *
		 * @note	Core thread.
         */
        virtual void resize(UINT32 width, UINT32 height) = 0;

        /**
         * @brief	Reposition the window.
		 *
		 * @note	Core thread.
         */
        virtual void move(INT32 left, INT32 top) = 0;

		/**
		 * @copydoc RenderTarget::isWindow.
		 */
		bool isWindow() const { return true; }

        /**
         * @brief	Indicates whether the window is visible (not minimized or obscured).
         */
        virtual bool isVisible() const { return true; }

        /** 
        * @copydoc RenderTarget::isActive
		*/
        virtual bool isActive() const { return mActive && isVisible(); }

        /** 
        * @brief	Indicates whether the window has been closed by the user.
		*/
        virtual bool isClosed() const = 0;
        
        /** 
        * @brief	Returns true if window is running in fullscreen mode.
		*/
        virtual bool isFullScreen() const;

		/**
		 * @brief	Returns true if the window is modal.
		 */
		bool isModal() const { return mDesc.modal; }

		/**
		 * @brief	Gets the horizontal origin of the window in pixels.
		 */
		INT32 getLeft() const { return mLeft; }

		/**
		 * @brief	Gets the vertical origin of the window in pixels.
		 */
		INT32 getTop() const { return mTop; }

		/**
		 * @brief	Indicates whether the window currently has keyboard focus.
		 */
		bool hasFocus() const { return mHasFocus; }

		/**
		 * @brief	Converts screen position into window local position.
		 */
		virtual Vector2I screenToWindowPos(const Vector2I& screenPos) const = 0;

		/**
		 * @brief	Converts window local position to screen position.
		 */
		virtual Vector2I windowToScreenPos(const Vector2I& windowPos) const = 0;

		/**
		 * @copydoc	RenderTarget::destroy
		 */
		virtual void destroy();

		/**
		 * @brief	Creates a new render window using the specified options. Optionally
		 *			makes the created window a child of another window.
		 */
		static RenderWindowPtr create(RENDER_WINDOW_DESC& desc, RenderWindowPtr parentWindow = nullptr);

    protected:
		friend class RenderWindowManager;

        RenderWindow(const RENDER_WINDOW_DESC& desc);

		/**
         * @brief	Called when window is moved or resized.
		 *
		 * @note	Core thread.
         */
        virtual void _windowMovedOrResized();

		/**
         * @brief	Called when window has received focus.
		 *
		 * @note	Core thread.
         */
		virtual void _windowFocusReceived();

		/**
         * @brief	Called when window has lost focus.
		 *
		 * @note	Core thread.
         */
		virtual void _windowFocusLost();
        
	protected:
		bool mIsFullScreen;
		INT32 mLeft;
		INT32 mTop;
		bool mHasFocus;
		bool mHidden;

		RENDER_WINDOW_DESC mDesc;
    };
}
