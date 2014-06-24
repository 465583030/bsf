#include "BsInput.h"
#include "BsTime.h"
#include "BsMath.h"
#include "BsRectI.h"
#include "BsDebug.h"
#include "BsRenderWindowManager.h"

using namespace std::placeholders;

namespace BansheeEngine
{
	const int Input::HISTORY_BUFFER_SIZE = 10; // Size of buffer used for input smoothing
	const float Input::WEIGHT_MODIFIER = 0.5f;

	Input::Input()
		:mSmoothHorizontalAxis(0.0f), mSmoothVerticalAxis(0.0f), mCurrentBufferIdx(0), mMouseLastRel(0, 0), mRawInputHandler(nullptr)
	{ 
		mHorizontalHistoryBuffer = bs_newN<float>(HISTORY_BUFFER_SIZE);
		mVerticalHistoryBuffer = bs_newN<float>(HISTORY_BUFFER_SIZE);
		mTimesHistoryBuffer = bs_newN<float>(HISTORY_BUFFER_SIZE);

		for(int i = 0; i < HISTORY_BUFFER_SIZE; i++)
		{
			mHorizontalHistoryBuffer[i] = 0.0f;
			mVerticalHistoryBuffer[i] = 0.0f;
			mTimesHistoryBuffer[i] = 0.0f;
		}

		for(int i = 0; i < BC_Count; i++)
			mKeyState[i] = ButtonState::Off;

		mOSInputHandler = bs_shared_ptr<OSInputHandler>();

		mOSInputHandler->onCharInput.connect(std::bind(&Input::charInput, this, _1));
		mOSInputHandler->onCursorMoved.connect(std::bind(&Input::cursorMoved, this, _1));
		mOSInputHandler->onCursorPressed.connect(std::bind(&Input::cursorPressed, this, _1));
		mOSInputHandler->onCursorReleased.connect(std::bind(&Input::cursorReleased, this, _1));
		mOSInputHandler->onDoubleClick.connect(std::bind(&Input::cursorDoubleClick, this, _1));
		mOSInputHandler->onInputCommand.connect(std::bind(&Input::inputCommandEntered, this, _1));

		RenderWindowManager::instance().onFocusGained.connect(std::bind(&Input::inputWindowChanged, this, _1));
	}

	Input::~Input()
	{
		bs_deleteN(mHorizontalHistoryBuffer, HISTORY_BUFFER_SIZE);
		bs_deleteN(mVerticalHistoryBuffer, HISTORY_BUFFER_SIZE);
		bs_deleteN(mTimesHistoryBuffer, HISTORY_BUFFER_SIZE);
	}

	void Input::_registerRawInputHandler(std::shared_ptr<RawInputHandler> inputHandler)
	{
		if(mRawInputHandler != inputHandler)
		{
			mRawInputHandler = inputHandler;

			if(mRawInputHandler != nullptr)
			{
				mRawInputHandler->onButtonDown.connect(std::bind(&Input::buttonDown, this, _1, _2));
				mRawInputHandler->onButtonUp.connect(std::bind(&Input::buttonUp, this, _1, _2));

				mRawInputHandler->onAxisMoved.connect(std::bind(&Input::axisMoved, this, _1, _2));
			}
		}
	}

	void Input::_update()
	{
		// Toggle states only remain active for a single frame before they are transitioned
		// into permanent state
		for(UINT32 i = 0; i < BC_Count; i++)
		{
			if(mKeyState[i] == ButtonState::ToggledOff)
				mKeyState[i] = ButtonState::Off;
			else if(mKeyState[i] == ButtonState::ToggledOn)
				mKeyState[i] = ButtonState::On;
		}

		if(mRawInputHandler == nullptr)
		{
			LOGERR("Raw input handler not initialized!");
			return;
		}
		else
			mRawInputHandler->_update();

		if(mOSInputHandler == nullptr)
		{
			LOGERR("OS input handler not initialized!");
			return;
		}
		else
			mOSInputHandler->_update();

		updateSmoothInput();
	}

	void Input::inputWindowChanged(RenderWindow& win)
	{
		if(mRawInputHandler != nullptr)
			mRawInputHandler->_inputWindowChanged(win);

		if(mOSInputHandler != nullptr)
			mOSInputHandler->_inputWindowChanged(win);
	}

	void Input::buttonDown(ButtonCode code, UINT64 timestamp)
	{
		mKeyState[code & 0x0000FFFF] = ButtonState::ToggledOn;

		if(!onButtonDown.empty())
		{
			ButtonEvent btnEvent;
			btnEvent.buttonCode = code;
			btnEvent.timestamp = timestamp;

			onButtonDown(btnEvent);
		}
	}

	void Input::buttonUp(ButtonCode code, UINT64 timestamp)
	{
		mKeyState[code & 0x0000FFFF] = ButtonState::ToggledOff;

		if(!onButtonUp.empty())
		{
			ButtonEvent btnEvent;
			btnEvent.buttonCode = code;
			btnEvent.timestamp = timestamp;

			onButtonUp(btnEvent);
		}
	}

	void Input::axisMoved(const RawAxisState& state, RawInputAxis axis)
	{
		if(axis == RawInputAxis::Mouse_XY)
			mMouseLastRel = Vector2I(-state.rel.x, -state.rel.y);

		mAxes[(int)axis] = state;
	}

	void Input::cursorMoved(const PointerEvent& event)
	{
		mMouseAbsPos = event.screenPos;

		if(!onPointerMoved.empty())
			onPointerMoved(event);
	}

	void Input::cursorPressed(const PointerEvent& event)
	{
		mMouseAbsPos = event.screenPos;

		if(!onPointerPressed.empty())
			onPointerPressed(event);
	}

	void Input::cursorReleased(const PointerEvent& event)
	{
		mMouseAbsPos = event.screenPos;

		if(!onPointerReleased.empty())
			onPointerReleased(event);
	}

	void Input::cursorDoubleClick(const PointerEvent& event)
	{
		if(!onPointerDoubleClick.empty())
			onPointerDoubleClick(event);
	}

	void Input::inputCommandEntered(InputCommandType commandType)
	{
		if(!onInputCommand.empty())
			onInputCommand(commandType);
	}

	void Input::charInput(UINT32 chr)
	{
		if(!onCharInput.empty())
		{
			TextInputEvent textInputEvent;
			textInputEvent.textChar = chr;

			onCharInput(textInputEvent);
		}
	}

	float Input::getHorizontalAxis() const
	{
		return mSmoothHorizontalAxis;
	}

	float Input::getVerticalAxis() const
	{
		return mSmoothVerticalAxis;
	}

	float Input::getAxisValue(AxisDevice device, AxisType type, UINT32 deviceIdx, bool smooth)
	{
		// TODO
		return 0.0f;
	}

	bool Input::isButtonHeld(ButtonCode button) const
	{
		return mKeyState[button & 0x0000FFFF] == ButtonState::On || mKeyState[button & 0x0000FFFF] == ButtonState::ToggledOn;
	}

	bool Input::isButtonUp(ButtonCode button) const
	{
		return mKeyState[button & 0x0000FFFF] == ButtonState::ToggledOff;
	}

	bool Input::isButtonDown(ButtonCode button) const
	{
		return mKeyState[button & 0x0000FFFF] == ButtonState::ToggledOn;
	}

	void Input::updateSmoothInput()
	{
		float currentTime = gTime().getTime();

		mHorizontalHistoryBuffer[mCurrentBufferIdx] = (float)mMouseLastRel.x;
		mVerticalHistoryBuffer[mCurrentBufferIdx] = (float)mMouseLastRel.y;
		mTimesHistoryBuffer[mCurrentBufferIdx] = currentTime;

		int i = 0;
		int idx = mCurrentBufferIdx;
		float currentWeight = 1.0f;
		float horizontalTotal = 0.0f;
		float verticalTotal = 0.0f;
		while(i < HISTORY_BUFFER_SIZE)
		{
			float timeWeight = 1.0f - (currentTime - mTimesHistoryBuffer[idx]) * 10.0f;
			if(timeWeight < 0.0f)
				timeWeight = 0.0f;

			horizontalTotal += mHorizontalHistoryBuffer[idx] * currentWeight * timeWeight;
			verticalTotal += mVerticalHistoryBuffer[idx] * currentWeight * timeWeight;

			currentWeight *= WEIGHT_MODIFIER;
			idx = (idx + 1) % HISTORY_BUFFER_SIZE;
			i++;
		}

		mCurrentBufferIdx = (mCurrentBufferIdx + 1) % HISTORY_BUFFER_SIZE;

		mSmoothHorizontalAxis = Math::clamp(horizontalTotal / HISTORY_BUFFER_SIZE, -1.0f, 1.0f);
		mSmoothVerticalAxis = Math::clamp(verticalTotal / HISTORY_BUFFER_SIZE, -1.0f, 1.0f);

		mMouseLastRel = Vector2I(0, 0);
	}

	Input& gInput()
	{
		return Input::instance();
	}
}