//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsModule.h"
#include "BsRenderWindow.h"
#include "BsEvent.h"

namespace BansheeEngine
{
	/** @addtogroup RenderAPI-Internal
	 *  @{
	 */

	/** Handles creation and internal updates relating to render windows. */
	class BS_CORE_EXPORT RenderWindowManager : public Module<RenderWindowManager>
	{
		/**	Holds information about a window that was moved or resized. */
		struct MoveOrResizeData
		{
			INT32 x, y;
			UINT32 width, height;
			RenderWindow* window;
		};

	public:
		RenderWindowManager();
		~RenderWindowManager();

		/**
		 * Creates a new render window using the specified options. Optionally makes the created window a child of another
		 * window.
		 */
		RenderWindowPtr create(RENDER_WINDOW_DESC& desc, RenderWindowPtr parentWindow);

		/** Called once per frame. Dispatches events. */
		void _update();

		/** Called by the core thread when window is destroyed. */
		void notifyWindowDestroyed(RenderWindow* window);

		/**	Called by the core thread when window receives focus. */
		void notifyFocusReceived(RenderWindowCore* window);

		/**	Called by the core thread when window loses focus. */
		void notifyFocusLost(RenderWindowCore* window);

		/**	Called by the core thread when window is moved or resized. */
		void notifyMovedOrResized(RenderWindowCore* window);

		/**	Called by the sim thread when window properties change. */
		void notifySyncDataDirty(RenderWindowCore* coreWindow);

		/**	Returns a list of all open render windows. */
		Vector<RenderWindow*> getRenderWindows() const;

		/** Event that is triggered when a window gains focus. */
		Event<void(RenderWindow&)> onFocusGained;

		/**	Event that is triggered when a window loses focus. */
		Event<void(RenderWindow&)> onFocusLost;

		/**	Event that is triggered when mouse leaves a window. */
		Event<void(RenderWindow&)> onMouseLeftWindow;
	protected:
		friend class RenderWindow;

		/**	Called by the core thread when mouse leaves a window. */
		void windowMouseLeft(RenderWindowCore* window);

		/**	Finds a sim thread equivalent of the provided core thread window implementation. */
		RenderWindow* getNonCore(const RenderWindowCore* window) const;

		/** @copydoc create */
		virtual RenderWindowPtr createImpl(RENDER_WINDOW_DESC& desc, UINT32 windowId, const RenderWindowPtr& parentWindow) = 0;

	protected:
		BS_MUTEX(mWindowMutex);
		Map<UINT32, RenderWindow*> mWindows;

		RenderWindow* mWindowInFocus;
		RenderWindow* mNewWindowInFocus;
		Vector<MoveOrResizeData> mMovedOrResizedWindows;
		Vector<RenderWindow*> mMouseLeftWindows;
		UnorderedSet<RenderWindow*> mDirtyProperties;
	};

	/**
	 * Handles creation and internal updates relating to render windows.
	 *
	 * @note	Core thread only.
	 */
	class BS_CORE_EXPORT RenderWindowCoreManager : public Module<RenderWindowCoreManager>
	{
	public:
		RenderWindowCoreManager();

		/** Creates a new render window using the specified options. */
		SPtr<RenderWindowCore> create(RENDER_WINDOW_DESC& desc);

		/** Called once per frame. Dispatches events. */
		void _update();

		/**	Called by the core thread when window properties change. */
		void notifySyncDataDirty(RenderWindowCore* window);

		/**	Returns a list of all open render windows. */
		Vector<RenderWindowCore*> getRenderWindows() const;

	protected:
		friend class RenderWindowCore;
		friend class RenderWindow;
		friend class RenderWindowManager;

		/** @copydoc create */
		virtual SPtr<RenderWindowCore> createInternal(RENDER_WINDOW_DESC& desc, UINT32 windowId) = 0;

		/**	Called whenever a window is created. */
		void windowCreated(RenderWindowCore* window);

		/**	Called by the core thread when window is destroyed. */
		void windowDestroyed(RenderWindowCore* window);

		BS_MUTEX(mWindowMutex);
		Vector<RenderWindowCore*> mCreatedWindows;
		UnorderedSet<RenderWindowCore*> mDirtyProperties;
		std::atomic_uint mNextWindowId;
	};

	/** @} */
}