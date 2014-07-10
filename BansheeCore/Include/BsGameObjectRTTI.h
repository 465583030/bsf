//__________________________ Banshee Project - A modern game development toolkit _________________________________//
//_____________________________________ www.banshee-project.com __________________________________________________//
//________________________ Copyright (c) 2014 Marko Pintera. All rights reserved. ________________________________//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsRTTIType.h"
#include "BsGameObject.h"
#include "BsSceneObject.h"
#include "BsGameObjectManager.h"

namespace BansheeEngine
{
	class BS_CORE_EXPORT GameObjectRTTI : public RTTIType<GameObject, IReflectable, GameObjectRTTI>
	{
	private:
		String& getName(GameObject* obj) { return obj->mName; }
		void setName(GameObject* obj, String& name) { obj->mName = name; }

		UINT64& getInstanceID(GameObject* obj) { return obj->mInstanceData->mInstanceId; }
		void setInstanceID(GameObject* obj, UINT64& instanceId) 
		{  
			// The system will have already assigned the instance ID, but since other objects might be referencing
			// the old (serialized) ID we store it in the GameObjectSerializationManager so we can map from old to new id.
			GameObjectManager::instance().registerDeserializedId(instanceId, obj->getInstanceId());
		}

	public:
		template <typename T>
		static std::shared_ptr<T> createGameObject()
		{
			return SceneObject::createEmptyComponent<T>();
		}

	public:
		GameObjectRTTI()
		{
			addPlainField("mInstanceID", 0, &GameObjectRTTI::getInstanceID, &GameObjectRTTI::setInstanceID);
			addPlainField("mName", 1, &GameObjectRTTI::getName, &GameObjectRTTI::setName);
		}

		virtual const String& getRTTIName()
		{
			static String name = "GameObject";
			return name;
		}

		virtual UINT32 getRTTIId()
		{
			return TID_GameObject;
		}

		virtual std::shared_ptr<IReflectable> newRTTIObject()
		{
			BS_EXCEPT(InternalErrorException, "Cannot instantiate an abstract class.");
		}
	};
}