//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsModule.h"

namespace BansheeEngine
{
	/**
	 * @brief	States the game in editor can be in.
	 */
	enum class PlayInEditorState
	{
		Stopped,
		Playing,
		Paused
	};

	/**
	 * @brief	Handles functionality specific to running the game in editor.
	 *
	 * @note	Play-in-editor functionality is only available for managed code, but can be easily extended to 
	 *			native code if needed (this would involve moving play in editor code into BansheeEngine library).
	 */
	class BS_SCR_BE_EXPORT PlayInEditorManager : public Module<PlayInEditorManager>
	{
	public:
		PlayInEditorManager();

		/**
		 * @brief	Returns the current play state of the game.
		 */
		PlayInEditorState getState() const { return mState; }

		/**
		 * @brief	Updates the play state of the game, making the game stop or start running.
		 */
		void setState(PlayInEditorState state);

		/**
		 * @brief	Gets the number of seconds that have elapsed since the game was started. This time does not
		 *			include time passed while the game is paused.
		 */
		float getPausableTime() const { return mPausableTime; }

		/**
		 * @brief	Runs the game for a single frame and then pauses it.
		 */
		void frameStep();

		/**
		 * @brief	Called once per frame.
		 *
		 * @note	Internal method.
		 */
		void update();

	private:
		/**
		 * @brief	Updates the play state of the game, making the game stop or start running. Unlike ::setState this
		 * 			will trigger the state change right away.
		 */
		void setStateImmediate(PlayInEditorState state);

		/**
		 * @brief	Saves the current state of the scene in memory.
		 */
		void saveSceneInMemory();

		PlayInEditorState mState;
		PlayInEditorState mNextState;
		bool mFrameStepActive;
		bool mScheduledStateChange;

		float mPausableTime;
		HSceneObject mSavedScene;
	};
}