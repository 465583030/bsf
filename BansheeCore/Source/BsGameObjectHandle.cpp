//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsCorePrerequisites.h"
#include "BsGameObject.h"
#include "BsGameObjectHandle.h"
#include "BsException.h"
#include "BsGameObject.h"
#include "BsGameObjectHandleRTTI.h"

namespace BansheeEngine
{
	GameObjectHandleBase::GameObjectHandleBase(const std::shared_ptr<GameObjectHandleData>& data)
		:mData(data)
	{ }

	GameObjectHandleBase::GameObjectHandleBase(const std::shared_ptr<GameObject> ptr)
	{
		mData = bs_shared_ptr_new<GameObjectHandleData>(ptr->mInstanceData);
	}

	GameObjectHandleBase::GameObjectHandleBase(std::nullptr_t ptr)
	{
		mData = bs_shared_ptr_new<GameObjectHandleData>(nullptr);
	}

	GameObjectHandleBase::GameObjectHandleBase()
	{
		mData = bs_shared_ptr_new<GameObjectHandleData>(nullptr);
	}

	void GameObjectHandleBase::_resolve(const GameObjectHandleBase& object) 
	{ 
		mData->mPtr = object.mData->mPtr;
	}

	void GameObjectHandleBase::_setHandleData(const GameObjectPtr& object)
	{
		mData->mPtr = object->mInstanceData;
	}

	void GameObjectHandleBase::throwIfDestroyed() const
	{
		if(isDestroyed()) 
		{
			BS_EXCEPT(InternalErrorException, "Trying to access an object that has been destroyed.");
		}
	}

	RTTITypeBase* GameObjectHandleBase::getRTTIStatic()
	{
		return GameObjectHandleRTTI::instance();
	}

	RTTITypeBase* GameObjectHandleBase::getRTTI() const
	{
		return GameObjectHandleBase::getRTTIStatic();
	}
}