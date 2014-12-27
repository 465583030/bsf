#include "BsRenderWindowManager.h"
#include "BsPlatform.h"

using namespace std::placeholders;

namespace BansheeEngine
{
	RenderWindowManager::RenderWindowManager()
		:mWindowInFocus(nullptr), mNewWindowInFocus(nullptr)
	{
		Platform::onMouseLeftWindow.connect(std::bind(&RenderWindowManager::windowMouseLeft, this, _1));
	}

	RenderWindowManager::~RenderWindowManager()
	{
		for (auto& dirtyPropertyEntry : mDirtyProperties)
		{
			DirtyPropertyData& dirtyPropertyData = dirtyPropertyEntry.second;

			if (dirtyPropertyEntry.second.data != nullptr)
				bs_delete(dirtyPropertyEntry.second.data);
		}

		mDirtyProperties.clear();
	}

	RenderWindowPtr RenderWindowManager::create(RENDER_WINDOW_DESC& desc, RenderWindowPtr parentWindow)
	{
		RenderWindowPtr renderWindow = createImpl(desc, parentWindow);
		renderWindow->_setThisPtr(renderWindow);
		renderWindow->initialize();

		{
			BS_LOCK_MUTEX(mWindowMutex);

			mCreatedWindows.push_back(renderWindow.get());
			mCoreToNonCoreMap[renderWindow->getCore().get()] = renderWindow.get();
		}
		
		return renderWindow;
	}

	void RenderWindowManager::notifyWindowDestroyed(RenderWindow* window)
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

			auto iterFind3 = mDirtyProperties.find(window);
			if (iterFind3 != mDirtyProperties.end())
			{
				if (iterFind3->second.data != nullptr)
					bs_delete(iterFind3->second.data);

				mDirtyProperties.erase(iterFind3);
			}

			mCoreToNonCoreMap.erase(window->getCore().get());
		}
	}

	void RenderWindowManager::notifyFocusReceived(RenderWindowCore* coreWindow)
	{
		RenderWindow* window = getNonCore(coreWindow);

		BS_LOCK_MUTEX(mWindowMutex);
		mNewWindowInFocus = window;

		setDirtyProperties(coreWindow);
	}

	void RenderWindowManager::notifyFocusLost(RenderWindowCore* coreWindow)
	{
		RenderWindow* window = getNonCore(coreWindow);

		BS_LOCK_MUTEX(mWindowMutex);
		mNewWindowInFocus = nullptr;

		setDirtyProperties(coreWindow);
	}

	void RenderWindowManager::notifyMovedOrResized(RenderWindowCore* coreWindow)
	{
		RenderWindow* window = getNonCore(coreWindow);

		bool isValidWindow = false;
		{
			BS_LOCK_MUTEX(mWindowMutex);

			isValidWindow = std::find(begin(mCreatedWindows), end(mCreatedWindows), window) != mCreatedWindows.end();
		}

		if(!isValidWindow)
			return;

		BS_LOCK_MUTEX(mWindowMutex);

		auto iterFind = std::find(begin(mMovedOrResizedWindows), end(mMovedOrResizedWindows), window);

		if (iterFind == end(mMovedOrResizedWindows))
			mMovedOrResizedWindows.push_back(window);

		setDirtyProperties(coreWindow);
	}

	void RenderWindowManager::notifyPropertiesDirty(RenderWindowCore* coreWindow)
	{
		RenderWindow* window = getNonCore(coreWindow);

		BS_LOCK_MUTEX(mWindowMutex);
		setDirtyProperties(coreWindow);
	}

	void RenderWindowManager::setDirtyProperties(RenderWindowCore* coreWindow)
	{
		RenderWindow* window = getNonCore(coreWindow);

		auto iterFind = mDirtyProperties.find(window);
		if (iterFind != mDirtyProperties.end())
		{
			if (iterFind->second.data != nullptr)
				bs_delete(iterFind->second.data);
		}

		mDirtyProperties[window] = DirtyPropertyData();
		DirtyPropertyData& dirtyPropertyData = mDirtyProperties[window];
		dirtyPropertyData.size = coreWindow->getSyncData(nullptr);
		dirtyPropertyData.data = (UINT8*)bs_alloc(dirtyPropertyData.size);

		coreWindow->getSyncData(dirtyPropertyData.data);
	}

	void RenderWindowManager::windowMouseLeft(RenderWindowCore* coreWindow)
	{
		BS_LOCK_MUTEX(mWindowMutex);

		RenderWindow* window = getNonCore(coreWindow);
		auto iterFind = std::find(begin(mMouseLeftWindows), end(mMouseLeftWindows), window);

		if (iterFind == end(mMouseLeftWindows))
			mMouseLeftWindows.push_back(window);
	}

	void RenderWindowManager::_update()
	{
		RenderWindow* newWinInFocus = nullptr;
		Vector<RenderWindow*> movedOrResizedWindows;
		Vector<RenderWindow*> mouseLeftWindows;

		{
			BS_LOCK_MUTEX(mWindowMutex);
			newWinInFocus = mNewWindowInFocus;

			movedOrResizedWindows = mMovedOrResizedWindows;
			mMovedOrResizedWindows.clear();

			mouseLeftWindows = mMouseLeftWindows;
			mMouseLeftWindows.clear();

			for (auto& dirtyPropertyEntry : mDirtyProperties)
			{
				RenderWindow* window = dirtyPropertyEntry.first;
				DirtyPropertyData& dirtyPropertyData = dirtyPropertyEntry.second;

				window->setSyncData(dirtyPropertyData.data, dirtyPropertyData.size);

				if (dirtyPropertyEntry.second.data != nullptr)
					bs_delete(dirtyPropertyEntry.second.data);
			}

			mDirtyProperties.clear();
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

		if (!onMouseLeftWindow.empty())
		{
			for (auto& window : mouseLeftWindows)
				onMouseLeftWindow(*window);
		}			
	}

	Vector<RenderWindow*> RenderWindowManager::getRenderWindows() const
	{
		BS_LOCK_MUTEX(mWindowMutex);

		return mCreatedWindows;
	}

	RenderWindow* RenderWindowManager::getNonCore(const RenderWindowCore* window) const
	{
		auto iterFind = mCoreToNonCoreMap.find(window);

		if (iterFind != mCoreToNonCoreMap.end())
			return iterFind->second;

		return nullptr;
	}

	SPtr<RenderWindowCore> RenderWindowCoreManager::create(RENDER_WINDOW_DESC& desc)
	{
		SPtr<RenderWindowCore> renderWindow = createInternal(desc);
		renderWindow->initialize();

		return renderWindow;
	}

	void RenderWindowCoreManager::windowCreated(RenderWindowCore* window)
	{
		BS_LOCK_MUTEX(mWindowMutex);

		mCreatedWindows.push_back(window);
	}

	void RenderWindowCoreManager::windowDestroyed(RenderWindowCore* window)
	{
		{
			BS_LOCK_MUTEX(mWindowMutex);

			auto iterFind = std::find(begin(mCreatedWindows), end(mCreatedWindows), window);

			if (iterFind == mCreatedWindows.end())
				BS_EXCEPT(InternalErrorException, "Trying to destroy a window that is not in the created windows list.");

			mCreatedWindows.erase(iterFind);
		}
	}

	Vector<RenderWindowCore*> RenderWindowCoreManager::getRenderWindows() const
	{
		BS_LOCK_MUTEX(mWindowMutex);

		return mCreatedWindows;
	}
}