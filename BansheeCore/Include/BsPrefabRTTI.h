#pragma once

#include "BsCorePrerequisites.h"
#include "BsRTTIType.h"
#include "BsPrefab.h"
#include "BsSceneObject.h"

namespace BansheeEngine
{
	class BS_CORE_EXPORT PrefabRTTI : public RTTIType < Prefab, Resource, PrefabRTTI >
	{
	private:
		SceneObjectPtr getSceneObject(Prefab* obj) { return obj->mRoot.getInternalPtr(); }
		void setSceneObject(Prefab* obj, SceneObjectPtr value) { obj->mRoot = value->getHandle(); }

		UINT32& getHash(Prefab* obj) { return obj->mHash; }
		void setHash(Prefab* obj, UINT32& val) { obj->mHash = val; }

	public:
		PrefabRTTI()
		{
			addReflectablePtrField("mRoot", 0, &PrefabRTTI::getSceneObject, &PrefabRTTI::setSceneObject);
			addPlainField("mHash", 1, &PrefabRTTI::getHash, &PrefabRTTI::setHash);
		}

		virtual const String& getRTTIName() override
		{
			static String name = "Prefab";
			return name;
		}

		virtual UINT32 getRTTIId() override
		{
			return TID_Prefab;
		}

		virtual std::shared_ptr<IReflectable> newRTTIObject() override
		{
			return Prefab::createEmpty();
		}
	};
}