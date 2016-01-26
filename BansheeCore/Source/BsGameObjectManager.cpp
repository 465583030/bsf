#include "BsGameObjectManager.h"
#include "BsGameObject.h"

namespace BansheeEngine
{
	GameObjectManager::GameObjectManager()
		:mNextAvailableID(1), mIsDeserializationActive(false)
	{

	}

	GameObjectManager::~GameObjectManager()
	{ }

	GameObjectHandleBase GameObjectManager::getObject(UINT64 id) const 
	{ 
		auto iterFind = mObjects.find(id);

		if(iterFind != mObjects.end())
			return iterFind->second;
		
		return nullptr;
	}

	bool GameObjectManager::tryGetObject(UINT64 id, GameObjectHandleBase& object) const
	{
		auto iterFind = mObjects.find(id);

		if(iterFind != mObjects.end())
		{
			object = iterFind->second;
			return true;
		}

		return false;
	}

	bool GameObjectManager::objectExists(UINT64 id) const 
	{ 
		return mObjects.find(id) != mObjects.end(); 
	}

	GameObjectHandleBase GameObjectManager::registerObject(const std::shared_ptr<GameObject>& object)
	{
		object->initialize(object, mNextAvailableID);

		GameObjectHandleBase handle(object);
		mObjects[mNextAvailableID] = handle;
		mNextAvailableID++;

		return handle;
	}

	void GameObjectManager::unregisterObject(const GameObjectHandleBase& object)
	{
		mObjects.erase(object->getInstanceId());
	}

	void GameObjectManager::startDeserialization()
	{
		assert(!mIsDeserializationActive);

		mIsDeserializationActive = true;
	}

	void GameObjectManager::endDeserialization()
	{
		assert(mIsDeserializationActive);

		for(auto& unresolvedHandle : mUnresolvedHandles)
		{
			UINT64 instanceId = unresolvedHandle.getInstanceId();

			auto findIter = mIdMapping.find(instanceId);
			if(findIter != mIdMapping.end())
			{
				instanceId = findIter->second;
			}

			auto findIterObj = mObjects.find(instanceId);

			if(findIterObj != mObjects.end())
				unresolvedHandle._resolve(findIterObj->second);	
			else
				unresolvedHandle._resolve(nullptr);
		}

		for(auto iter = mEndCallbacks.rbegin(); iter != mEndCallbacks.rend(); ++iter)
		{
			(*iter)();
		}

		mIsDeserializationActive = false;
		mActiveDeserializedObject = nullptr;
		mIdMapping.clear();
		mUnresolvedHandles.clear();
		mEndCallbacks.clear();
	}

	void GameObjectManager::registerDeserializedId(UINT64 serializedId, UINT64 actualId)
	{
#if BS_DEBUG_MODE
		if(!mIsDeserializationActive)
		{
			BS_EXCEPT(InvalidStateException, "ID mapping may only be modified while deserialization is active.");
		}
#endif

		mIdMapping[serializedId] = actualId;
	}

	void GameObjectManager::registerUnresolvedHandle(const GameObjectHandleBase& object)
	{
#if BS_DEBUG_MODE
		if(!mIsDeserializationActive)
		{
			BS_EXCEPT(InvalidStateException, "Unresolved handle queue only be modified while deserialization is active.");
		}
#endif

		mUnresolvedHandles.push_back(object);
	}

	void GameObjectManager::registerOnDeserializationEndCallback(std::function<void()> callback)
	{
#if BS_DEBUG_MODE
		if(!mIsDeserializationActive)
		{
			BS_EXCEPT(InvalidStateException, "Callback queue only be modified while deserialization is active.");
		}
#endif

		mEndCallbacks.push_back(callback);
	}
}