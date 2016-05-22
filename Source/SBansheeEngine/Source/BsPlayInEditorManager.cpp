//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsPlayInEditorManager.h"
#include "BsScriptGameObjectManager.h"
#include "BsTime.h"
#include "BsSceneManager.h"
#include "BsSceneObject.h"
#include "BsApplication.h"
#include "BsPhysics.h"
#include "BsAudio.h"

namespace BansheeEngine
{
	PlayInEditorManager::PlayInEditorManager()
		:mState(PlayInEditorState::Stopped), mNextState(PlayInEditorState::Stopped), 
		mFrameStepActive(false), mScheduledStateChange(false), mPausableTime(0.0f)
	{
		if (!gApplication().isEditor())
			mState = PlayInEditorState::Playing;
	}

	void PlayInEditorManager::setState(PlayInEditorState state)
	{
		if (!gApplication().isEditor())
			return;

		// Delay state change to next frame as this method could be called in middle of object update, in which case
		// part of the objects before this call would receive different state than other objects.
		mScheduledStateChange = true;
		mNextState = state;
	}

	void PlayInEditorManager::setStateImmediate(PlayInEditorState state)
	{
		if (mState == state)
			return;

		PlayInEditorState oldState = mState;
		mState = state;

		switch (state)
		{
		case PlayInEditorState::Stopped:
		{
			mFrameStepActive = false;
			mPausableTime = 0.0f;

			gPhysics().setPaused(true);
			gAudio().setPaused(true);

			mSavedScene->_instantiate();
			gSceneManager()._setRootNode(mSavedScene);
			mSavedScene = nullptr;
		}
			break;
		case PlayInEditorState::Playing:
		{
			if (oldState == PlayInEditorState::Stopped)
			{
				saveSceneInMemory();
				ScriptGameObjectManager::instance().wakeRuntimeComponents();
			}

			gPhysics().setPaused(false);
			gAudio().setPaused(false);
		}
			break;
		case PlayInEditorState::Paused:
		{
			mFrameStepActive = false;
			gPhysics().setPaused(true);
			gAudio().setPaused(true);

			if (oldState == PlayInEditorState::Stopped)
			{
				saveSceneInMemory();
				ScriptGameObjectManager::instance().wakeRuntimeComponents();
			}
		}
			break;
		default:
			break;
		}		
	}

	void PlayInEditorManager::frameStep()
	{
		if (!gApplication().isEditor())
			return;

		switch (mState)
		{
		case PlayInEditorState::Stopped:
		case PlayInEditorState::Paused:
			setState(PlayInEditorState::Playing);
			break;
		default:
			break;
		}

		mFrameStepActive = true;
	}

	void PlayInEditorManager::update()
	{
		if (mState == PlayInEditorState::Playing)
			mPausableTime += gTime().getFrameDelta();

		if (mScheduledStateChange)
		{
			setStateImmediate(mNextState);
			mScheduledStateChange = false;
		}

		if (mFrameStepActive)
		{
			setState(PlayInEditorState::Paused);
			mFrameStepActive = false;
		}
	}

	void PlayInEditorManager::saveSceneInMemory()
	{
		mSavedScene = SceneManager::instance().getRootNode()->clone(false);

		// Remove objects with "dont save" flag
		Stack<HSceneObject> todo;
		todo.push(mSavedScene);

		while (!todo.empty())
		{
			HSceneObject current = todo.top();
			todo.pop();

			if (current->hasFlag(SOF_DontSave))
				current->destroy();
			else
			{
				UINT32 numChildren = current->getNumChildren();
				for (UINT32 i = 0; i < numChildren; i++)
					todo.push(current->getChild(i));
			}
		}
	}
}