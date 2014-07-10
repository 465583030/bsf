//__________________________ Banshee Project - A modern game development toolkit _________________________________//
//_____________________________________ www.banshee-project.com __________________________________________________//
//________________________ Copyright (c) 2014 Marko Pintera. All rights reserved. ________________________________//
#include "BsCorePrerequisites.h"
#include "BsGameObject.h"
#include "BsGameObjectHandle.h"
#include "BsException.h"
#include "BsGameObjectHandleRTTI.h"

namespace BansheeEngine
{
	GameObjectHandleBase::GameObjectHandleBase(const std::shared_ptr<GameObjectHandleData>& data)
		:mData(data)
	{ }

	GameObjectHandleBase::GameObjectHandleBase(const std::shared_ptr<GameObject> ptr)
	{
		mData = bs_shared_ptr<GameObjectHandleData, PoolAlloc>(ptr->mInstanceData);
	}

	GameObjectHandleBase::GameObjectHandleBase(std::nullptr_t ptr)
	{
		mData = bs_shared_ptr<GameObjectHandleData, PoolAlloc>(nullptr);
	}

	GameObjectHandleBase::GameObjectHandleBase()
	{
		mData = bs_shared_ptr<GameObjectHandleData, PoolAlloc>(nullptr);
	}

	void GameObjectHandleBase::_resolve(const GameObjectHandleBase& object) 
	{ 
		mData->mPtr = object.mData->mPtr;
		mData->mInstanceId = object.mData->mInstanceId;
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