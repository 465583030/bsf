#pragma once

#include "BsCorePrerequisites.h"
#include "BsModule.h"
#include "BsRectI.h"
#include "BsOSInputHandler.h"
#include "BsRawInputHandler.h"
#include "BsInputFwd.h"

namespace BansheeEngine
{
	/**
	 * @brief	Primary module used for dealing with input. Allows you to receieve 
	 *			and query raw or OS input for mouse/keyboard/gamepad.
	 *
	 *			All inputs are received through an input handler, which can be overriden to 
	 *			provide custom input functionality.
	 */
	class BS_CORE_EXPORT Input : public Module<Input>
	{
		/**
		 * @brief	Possible button states
		 */
		enum class ButtonState
		{
			Off, /**< Button is not being pressed. */
			On, /**< Button is being pressed. */
			ToggledOn, /**< Button has been pressed this frame. */
			ToggledOff /**< Button has been released this frame. */
		};

	public:
		Input();
		~Input();

		/**
		 * @brief	Triggered whenever a button is first pressed.
		 */
		Event<void(const ButtonEvent&)> onButtonDown;

		/**
		 * @brief	Triggered whenever a button is first released.
		 */
		Event<void(const ButtonEvent&)> onButtonUp;

		/**
		 * @brief	Triggered whenever user inputs a text character. 
		 */
		Event<void(const TextInputEvent&)> onCharInput;

		/**
		 * @brief	Triggers when some pointing device (mouse cursor, touch) moves.
		 */
		Event<void(const PointerEvent&)> onPointerMoved;

		/**
		 * @brief	Triggers when some pointing device (mouse cursor, touch) button is pressed.
		 */
		Event<void(const PointerEvent&)> onPointerPressed;

		/**
		 * @brief	Triggers when some pointing device (mouse cursor, touch) button is released.
		 */
		Event<void(const PointerEvent&)> onPointerReleased;

		/**
		 * @brief	Triggers when some pointing device (mouse cursor, touch) button is double clicked.
		 */
		Event<void(const PointerEvent&)> onPointerDoubleClick;

		// TODO Low priority: Remove this, I can emulate it using virtual input
		/**
		 * @brief	Triggers on special input commands.
		 */
		Event<void(InputCommandType)> onInputCommand;

		/**
		 * @brief	Registers a new input handler. Replaces any previous input handler.
		 *
		 * @note	Internal method.
		 */
		void _registerRawInputHandler(std::shared_ptr<RawInputHandler> inputHandler);

		/**
		 * @brief	Called every frame. Dispatches any callbacks resulting from input by the user.
		 *
		 * @note	Internal method.
		 */
		void _update();

		/**
		 * @brief	Returns smoothed mouse/joystick input in the horizontal axis.
		 *
		 * @return	The horizontal axis value ranging [-1.0f, 1.0f].
		 */
		float getHorizontalAxis() const;

		/**
		 * @brief	Returns smoothed mouse/joystick input in the vertical axis.
		 *
		 * @return	The vertical axis value ranging [-1.0f, 1.0f].
		 */
		float getVerticalAxis() const;

		/**
		 * @brief	Returns value of the specified input axis in range [-1.0, 1.0].
		 *
		 * @param	device		Device from which to query the axis.
		 * @param	type		Type of axis to query.
		 * @param	deviceIdx	Index of the device in case more than one is hooked up.
		 * @param	smooth		Should the returned value be smoothed.
		 */
		float getAxisValue(AxisDevice device, AxisType type, UINT32 deviceIdx = 0, bool smooth = false);

		/**
		 * @brief	Query if the provided button is currently being held (this frame or previous frames).
		 */
		bool isButtonHeld(ButtonCode keyCode) const;

		/**
		 * @brief	Query if the provided button is currently being released (one true for one frame).
		 */
		bool isButtonUp(ButtonCode keyCode) const;

		/**
		 * @brief	Query if the provided button is currently being pressed (one true for one frame).
		 */
		bool isButtonDown(ButtonCode keyCode) const;

		/**
		 * @brief	Returns mouse cursor position.
		 */
		Vector2I getCursorPosition() const { return mMouseAbsPos; }
	private:
		std::shared_ptr<RawInputHandler> mRawInputHandler;
		std::shared_ptr<OSInputHandler> mOSInputHandler;

		float mSmoothHorizontalAxis;
		float mSmoothVerticalAxis;

		float* mHorizontalHistoryBuffer;
		float* mVerticalHistoryBuffer;
		float* mTimesHistoryBuffer;
		int	mCurrentBufferIdx;

		Vector2I mMouseLastRel;
		Vector2I mMouseAbsPos;

		RawAxisState mAxes[RawInputAxis::Count];
		ButtonState mKeyState[BC_Count];

		void buttonDown(ButtonCode code, UINT64 timestamp);
		void buttonUp(ButtonCode code, UINT64 timestamp);

		void charInput(UINT32 chr);

		/**
		 * @brief	Raw mouse/joystick axis input.
		 */
		void axisMoved(const RawAxisState& state, RawInputAxis axis);

		/**
		 * @brief	Cursor movement as OS reports it. Used for screen cursor position.
		 */
		void cursorMoved(const PointerEvent& event);

		/**
		 * @brief	Cursor button presses as OS reports it. 
		 */
		void cursorPressed(const PointerEvent& event);

		/**
		 * @brief	Cursor button releases as OS reports it.
		 */
		void cursorReleased(const PointerEvent& event);
		
		/**
		 * @brief	Cursor button releases as OS reports it.
		 */
		void cursorDoubleClick(const PointerEvent& event);

		/**
		 * @brief	Input commands as OS reports them.
		 */
		void inputCommandEntered(InputCommandType commandType);

		/**
		 * @brief	Updates the axis input values that need smoothing.
		 */
		void updateSmoothInput();

		/**
		 * @brief	Called when window in focus changes, as reported by the OS.
		 */
		void inputWindowChanged(RenderWindow& win);

		/************************************************************************/
		/* 								STATICS		                      		*/
		/************************************************************************/
		static const int HISTORY_BUFFER_SIZE; // Size of buffer used for input smoothing
		static const float WEIGHT_MODIFIER;
	};

	/**
	 * @copydoc	Input
	 */
	BS_CORE_EXPORT Input& gInput();
}