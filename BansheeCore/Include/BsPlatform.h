#pragma once

#include "BsCorePrerequisites.h"
#include "BsInputFwd.h"
#include "BsVector2I.h"
#include "BsRect2I.h"
#include "BsEvent.h"

namespace BansheeEngine
{
	/**
	 * @brief	Contains values representing default
	 *			mouse cursor types.
	 */
	enum class PlatformCursorType
	{
		Arrow,
		Wait,
		IBeam,
		Help,
		Hand,
		SizeAll,
		SizeNESW,
		SizeNS,
		SizeNWSE,
		SizeWE
	};

	/**
	 * @brief	Contains values reprenting window non client areas.
	 *
	 * @note	These are used for things like resize/move and tell the OS
	 *			where each of those areas are on our window.
	 */
	enum class NonClientAreaBorderType
	{
		TopLeft,
		Top,
		TopRight,
		Left,
		Right,
		BottomLeft,
		Bottom,
		BottomRight
	};

	/**
	 * @brief	Types of mouse buttons provided by the OS.
	 */
	enum class OSMouseButton
	{
		Left, Middle, Right, Count
	};

	/**
	 * @brief	Describes pointer (mouse, touch) states as reported
	 *			by the OS.
	 */
	struct BS_CORE_EXPORT OSPointerButtonStates
	{
		OSPointerButtonStates()
		{
			mouseButtons[0] = false;
			mouseButtons[1] = false;
			mouseButtons[2] = false;

			shift = false;
			ctrl = false;
		}

		bool mouseButtons[OSMouseButton::Count];
		bool shift, ctrl;
	};

	/**
	 * @brief	Type of drop event type. This is used
	 *			when dragging items over drop targets.
	 */
	enum class OSDropType
	{
		FileList,
		None
	};

	/**
	 * @brief	Drop targets allow you to register a certain portion of a window as a drop target that
	 *			accepts certain drop types from the OS (platform) specific drag and drop system. Accepted 
	 *			drop types are provided by the OS and include things like file and item dragging.
	 *
	 *			You will receive events with the specified drop area as long as it is active.
	 */
	class BS_CORE_EXPORT OSDropTarget
	{
	public:
		/**
		 * @brief	Triggered when a pointer is being dragged over the drop area.
		 *			Provides window coordinates of the pointer position.
		 */
		Event<void(INT32 x, INT32 y)> onDragOver;

		/**
		 * @brief	Triggered when the user completes a drop while pointer is over
		 *			the drop area.
		 *			Provides window coordinates of the pointer position.
		 */
		Event<void(INT32 x, INT32 y)> onDrop;

		/**
		 * @brief	Triggered when a pointer enters the drop area.
		 *			Provides window coordinates of the pointer position.
		 */
		Event<void(INT32 x, INT32 y)> onEnter;

		/**
		 * @brief	Triggered when a pointer leaves the drop area.
		 */
		Event<void()> onLeave;

		/**
		 * @brief	Sets the drop target area, in local window coordinates.
		 */
		void setArea(INT32 x, INT32 y, UINT32 width, UINT32 height);

		/**
		 * @brief	Gets the type of drop that this drop target is looking for. Only
		 *			valid after a drop has been triggered.
		 */
		OSDropType getDropType() const { return mDropType; }

		/**
		 * @brief	Returns a list of files received by the drop target. Only valid
		 *			after a drop of FileList type has been triggered.
		 */
		const Vector<WString>& getFileList() const { return *mFileList; }

		/**
		 * @brief	Internal method. Clears all internal values.
		 */
		void _clear();

		/**
		 * @brief	Internal method. Sets the file list and marks the drop event as FileList.
		 */
		void _setFileList(const Vector<WString>& fileList);

		/**
		 * @brief	Marks the drop area as inactive or active.
		 */
		void _setActive(bool active) { mActive = active; }

		/**
		 * @brief	Checks is the specified position within the current drop area.
		 *			Position should be in window local coordinates.
		 */
		bool _isInside(const Vector2I& pos) const;

		/**
		 * @brief	Returns true if the drop target is active.
		 */
		bool _isActive() const { return mActive; }
	private:
		friend class Platform;

		OSDropTarget(const RenderWindow* ownerWindow, INT32 x, INT32 y, UINT32 width, UINT32 height);
		~OSDropTarget();

		/**
		 * @brief	Returns a render window this drop target is attached to.
		 */
		const RenderWindow* getOwnerWindow() const { return mOwnerWindow; }
	private:
		INT32 mX, mY;
		UINT32 mWidth, mHeight;
		bool mActive;
		const RenderWindow* mOwnerWindow;

		OSDropType mDropType;

		union
		{
			Vector<WString>* mFileList;
		};
	};

	/**
	 * @brief	Represents a specific non client area used for window resizing.
	 */
	struct BS_CORE_EXPORT NonClientResizeArea
	{
		NonClientAreaBorderType type;
		Rect2I area;
	};

	/**
	 * @brief	Contains a list of window move and resize non client areas.
	 */
	struct BS_CORE_EXPORT WindowNonClientAreaData
	{
		Vector<NonClientResizeArea> resizeAreas;
		Vector<Rect2I> moveAreas;
	};

	/**
	 * @brief	Provides access to various operating system functions, including
	 * 			the main message pump.
	 */
	class BS_CORE_EXPORT Platform
	{
	public:
		struct Pimpl;

		Platform() { }
		virtual ~Platform();

		/**
		 * @brief	Retrieves the cursor position in screen coordinates.
		 *
		 * @note	Thread safe.
		 */
		static Vector2I getCursorPosition();

		/**
		 * @brief	Moves the cursor to the specified screen position.
		 *
		 * @note	Thread safe.
		 */
		static void setCursorPosition(const Vector2I& screenPos);

		/**
		 * @brief	Capture mouse to this window so that we get mouse input even if the mouse leaves the window area.
		 *
		 * @note	Thread safe.
		 */
		static void captureMouse(const RenderWindow& window);

		/**
		 * @brief	Releases the mouse capture set by "captureMouse"
		 *
		 * @note	Thread safe.
		 */
		static void releaseMouseCapture();

		/**
		 * @brief	Checks if provided over screen position is over the specified window.
		 */
		static bool isPointOverWindow(const RenderWindow& window, const Vector2I& screenPos);

		/**
		 * @brief	Limit cursor movement to the specified window.
		 *
		 * @note	Thread safe.
		 */
		static void clipCursorToWindow(const RenderWindow& window);

		/**
		 * @brief	Clip cursor to specific area on the screen.
		 *
		 * @note	Thread safe.
		 */

		static void clipCursorToRect(const Rect2I& screenRect);

		/**
		 * @brief	Disables cursor clipping.
		 *
		 * @note	Thread safe.
		 */
		static void clipCursorDisable();

		/**
		 * @brief	Hides the cursor.
		 *
		 * @note	Thread safe.
		 */
		static void hideCursor();

		/**
		 * @brief	Shows the cursor.
		 *
		 * @note	Thread safe.
		 */
		static void showCursor();

		/**
		 * @brief	Query if the cursor is hidden.
		 *
		 * @note	Thread safe.
		 */
		static bool isCursorHidden();

		/**
		 * @brief	Sets a cursor using a custom image.
		 *
		 * @param 	pixelData	Cursor image data.
		 * @param	hotSpot		Offset on the cursor image to where the actual input happens (e.g. tip of the Arrow cursor).
		 *
		 * @note	Thread safe.
		 */
		static void setCursor(PixelData& pixelData, const Vector2I& hotSpot);

		/**
		 * @brief	Sets an icon for the main application window.
		 *
		 * @param 	pixelData	Icon image data. This will be resized to the required icon size, depending on platform
		 * 						implementation.
		 *
		 * @note	Thread safe.
		 */
		static void setIcon(const PixelData& pixelData);

		/**
		 * @brief	Sets custom caption non client areas for the specified window. Using custom client
		 * 			areas will override window move/drag operation and trigger when user interacts
		 * 			with the custom area.
		 *
		 * @note	Thread safe.
		 * 			All provided areas are relative to the specified window.
		 * 			Mostly useful for frameless windows that don't have typical caption bar.
		 */
		static void setCaptionNonClientAreas(const RenderWindowCore& window, const Vector<Rect2I>& nonClientAreas);

		/**
		 * @brief	Sets custom non client areas for the specified window. Using custom client
		 * 			areas will override window resize operation and trigger when user interacts
		 * 			with the custom area.
		 *
		 * @note	Thread safe.
		 * 			All provided areas are relative to the specified window.
		 * 			Mostly useful for frameless windows that don't have typical border.
		 */
		static void setResizeNonClientAreas(const RenderWindowCore& window, const Vector<NonClientResizeArea>& nonClientAreas);

		/**
		 * @brief	Resets the non client areas for the specified windows and allows
		 * 			the platform to use the default values.
		 *
		 * @note	Thread safe.
		 */
		static void resetNonClientAreas(const RenderWindowCore& window);

		/**
		 * @brief	Creates a drop target that you can use for tracking OS drag and drop operations performed over
		 * 			a certain area on the specified window.
		 *
		 * @param	window	The window on which to track drop operations.
		 * @param	x	  	The x coordinate of the area to track, relative to window.
		 * @param	y	  	The y coordinate of the area to track, relative to window.
		 * @param	width 	The width of the area to track.
		 * @param	height	The height of the area to track.
		 *
		 * @return	OSDropTarget that you will use to receive all drop data. When no longer needed make sure to destroy it with
		 * 			destroyDropTarget().
		 */
		static OSDropTarget& createDropTarget(const RenderWindow* window, int x, int y, unsigned int width, unsigned int height);

		/**
		 * @brief	Destroys a drop target previously created with createDropTarget.
		 */
		static void destroyDropTarget(OSDropTarget& target);

		/**
		 * @brief	Message pump. Processes OS messages and returns when it's free.
		 *
		 * @note	Internal method.
		 * 			Core thread only.
		 */
		static void _messagePump();

		/**
		 * @brief	Called during application start up from the sim thread.
		 * 			Must be called before any other operations are done.
		 *
		 * @note	Internal method.
		 */
		static void _startUp();

		/**
		 * @brief	Called once per frame from the sim thread.
		 *
		 * @note	Internal method.
		 * 			Sim thread only.
		 */
		static void _update();

		/**
		 * @brief	Called once per frame from the core thread.
		 *
		 * @note	Internal method.
		 * 			Core thread only.
		 */
		static void _coreUpdate();

		/**
		 * @brief	Called during application shut down from the sim thread.
		 *
		 * @note	Internal method.
		 * 			Sim thread only.
		 */
		static void _shutDown();

		/**
		 * @brief	Triggered when a pointer leaves the provided window.
		 *
		 * @note	Sim thread only.
		 */
		static Event<void(RenderWindowCore*)> onMouseLeftWindow;

		/**
		 * @brief	Triggered whenever the pointer moves.
		 *
		 * @note	Core thread only.
		 */
		static Event<void(const Vector2I&, OSPointerButtonStates)> onCursorMoved;

		/**
		 * @brief	Triggered whenever a pointer button is pressed.
		 *
		 * @note	Core thread only.
		 */
		static Event<void(const Vector2I&, OSMouseButton button, OSPointerButtonStates)> onCursorButtonPressed;

		/**
		 * @brief	Triggered whenever pointer button is released.
		 *
		 * @note	Core thread only.
		 */
		static Event<void(const Vector2I&, OSMouseButton button, OSPointerButtonStates)> onCursorButtonReleased;

		/**
		 * @brief	Triggered whenever a pointer button is double clicked.
		 *
		 * @note	Core thread only.
		 */
		static Event<void(const Vector2I&, OSPointerButtonStates)> onCursorDoubleClick;

		/**
		 * @brief	Triggered whenever an input command is entered.
		 *
		 * @note	Core thread only.
		 */
		static Event<void(InputCommandType)> onInputCommand;

		/**
		 * @brief	Triggered whenever the mouse wheel is scolled.
		 *
		 * @note	Core thread only.
		 */
		static Event<void(float)> onMouseWheelScrolled;

		/**
		 * @brief	Triggered whenever a character is entered.
		 *
		 * @note	Core thread only.
		 */
		static Event<void(UINT32)> onCharInput;

		/**
		 * @brief	Triggered whenever mouse capture state for the window is changed
		 * 			(it receives or loses it).
		 *
		 * @note	Core thread only.
		 */
		static Event<void()> onMouseCaptureChanged;
	protected:
		static Pimpl* mData;
	};
}