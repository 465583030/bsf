//__________________________ Banshee Project - A modern game development toolkit _________________________________//
//_____________________________________ www.banshee-project.com __________________________________________________//
//________________________ Copyright (c) 2014 Marko Pintera. All rights reserved. ________________________________//
#include "BsRenderWindowManager.h"
#include "BsPlatform.h"

using namespace std::placeholders;

namespace BansheeEngine
{
	RenderWindowManager::RenderWindowManager()
		:mWindowInFocus(nullptr), mNewWindowInFocus(nullptr)
	{
		Platform::onWindowFocusReceived.connect(std::bind(&RenderWindowManager::windowFocusReceived, this, _1));
		Platform::onWindowFocusLost.connect(std::bind(&RenderWindowManager::windowFocusLost, this, _1));
		Platform::onWindowMovedOrResized.connect(std::bind(&RenderWindowManager::windowMovedOrResized, this, _1));
	}

	RenderWindowPtr RenderWindowManager::create(RENDER_WINDOW_DESC& desc, RenderWindowPtr parentWindow)
	{
		RenderWindowPtr renderWindow = createImpl(desc, parentWindow);
		renderWindow->_setThisPtr(renderWindow);
		renderWindow->initialize();

		{
			BS_LOCK_MUTEX(mWindowMutex);

			mCreatedWindows.push_back(renderWindow.get());
		}
		
		return renderWindow;
	}

	void RenderWindowManager::windowDestroyed(RenderWindow* window)
	{
		{
			BS_LOCK_MUTEX(mWindowMutex);

			auto iterFind = std::find(begin(mCreatedWindows), end(mCreatedWindows), window);

			if(iterFind == mCreatedWindows.end())
				BS_EXCEPT(InternalErrorException, "Trying to destroy a window that is not in the created windows list.");

			mCreatedWindows.erase(iterFind);

			auto iterFind2 = std::find(begin(mMovedOrResizedWindows), end(mMovedOrResizedWindows), window);

			if(iterFind2 != mMovedOrResizedWindows.end())
				mMovedOrResizedWindows.erase(iterFind2);
		}
	}

	void RenderWindowManager::windowFocusReceived(RenderWindow* window)
	{
		window->_windowFocusReceived();

		BS_LOCK_MUTEX(mWindowMutex);
		mNewWindowInFocus = window;
	}

	void RenderWindowManager::windowFocusLost(RenderWindow* window)
	{
		window->_windowFocusLost();

		BS_LOCK_MUTEX(mWindowMutex);
		mNewWindowInFocus = nullptr;
	}

	void RenderWindowManager::windowMovedOrResized(RenderWindow* window)
	{
		bool isValidWindow = false;
		{
			BS_LOCK_MUTEX(mWindowMutex);

			isValidWindow = std::find(begin(mCreatedWindows), end(mCreatedWindows), window) != mCreatedWindows.end();
		}

		if(!isValidWindow)
			return;

		window->_windowMovedOrResized();

		BS_LOCK_MUTEX(mWindowMutex);

		auto iterFind = std::find(begin(mMovedOrResizedWindows), end(mMovedOrResizedWindows), window);

		if(iterFind == end(mMovedOrResizedWindows))
			mMovedOrResizedWindows.push_back(window);
	}

	void RenderWindowManager::_update()
	{
		RenderWindow* newWinInFocus = nullptr;
		Vector<RenderWindow*> movedOrResizedWindows;

		{
			BS_LOCK_MUTEX(mWindowMutex);
			newWinInFocus = mNewWindowInFocus;

			movedOrResizedWindows = mMovedOrResizedWindows;
			mMovedOrResizedWindows.clear();
		}

		if(mWindowInFocus != newWinInFocus)
		{
			if(mWindowInFocus != nullptr)
			{
				if(!onFocusLost.empty())
					onFocusLost(*mWindowInFocus);
			}

			if(newWinInFocus != nullptr)
			{
				if(!onFocusGained.empty())
					onFocusGained(*newWinInFocus);
			}

			mWindowInFocus = newWinInFocus;
		}

		for(auto& window : movedOrResizedWindows)
		{
			if(!window->onResized.empty())
				window->onResized();
		}
	}

	Vector<RenderWindow*> RenderWindowManager::getRenderWindows() const
	{
		BS_LOCK_MUTEX(mWindowMutex);

		return mCreatedWindows;
	}
}