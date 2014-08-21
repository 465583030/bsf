#include "BsRenderWindow.h"
#include "BsCoreThread.h"
#include "BsRenderWindowManager.h"
#include "BsViewport.h"
#include "BsPlatform.h"

namespace BansheeEngine 
{
    RenderWindow::RenderWindow(const RENDER_WINDOW_DESC& desc)
		: RenderTarget(), mIsFullScreen(false), mDesc(desc), mHasFocus(false), mLeft(0), mTop(0), mHidden(false)
    {
		mWidth = desc.videoMode.getWidth();
		mHeight = desc.videoMode.getHeight();
		mHwGamma = desc.gamma;
		mVSync = desc.vsync;
		mMultisampleCount = desc.multisampleCount;
		mLeft = desc.left; 
		mTop = desc.top;
		mIsFullScreen = desc.fullscreen;
		mHidden = desc.hidden;
    }

	RenderWindow::~RenderWindow() 
	{
		
	}

	void RenderWindow::setHidden(bool hidden)
	{
		THROW_IF_NOT_CORE_THREAD;
	}

    bool RenderWindow::isFullScreen(void) const
    {
        return mIsFullScreen;
    }

	void RenderWindow::_windowMovedOrResized()
	{
		THROW_IF_NOT_CORE_THREAD;
	}

	void RenderWindow::_windowFocusReceived()
	{
		THROW_IF_NOT_CORE_THREAD;

		mHasFocus = true;
	}

	void RenderWindow::_windowFocusLost()
	{
		THROW_IF_NOT_CORE_THREAD;

		mHasFocus = false;
	}

	void RenderWindow::destroy()
	{
		Platform::resetNonClientAreas(*this);

		RenderWindowManager::instance().windowDestroyed(this);

		RenderTarget::destroy();
	}

	RenderWindowPtr RenderWindow::create(RENDER_WINDOW_DESC& desc, RenderWindowPtr parentWindow)
	{
		return RenderWindowManager::instance().create(desc, parentWindow);
	}
}
