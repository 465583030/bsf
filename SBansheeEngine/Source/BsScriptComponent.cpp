#include "BsScriptComponent.h"
#include "BsScriptGameObjectManager.h"
#include "BsScriptObjectManager.h"
#include "BsScriptAssemblyManager.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"
#include "BsMonoManager.h"
#include "BsMonoUtil.h"
#include "BsScriptSceneObject.h"
#include "BsManagedComponent.h"
#include "BsSceneObject.h"
#include "BsMonoUtil.h"

namespace BansheeEngine
{
	ScriptComponent::ScriptComponent(MonoObject* instance)
		:ScriptObject(instance), mTypeMissing(false)
	{ 
		assert(instance != nullptr);

		MonoUtil::getClassName(instance, mNamespace, mType);
	}

	void ScriptComponent::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_AddComponent", &ScriptComponent::internal_addComponent);
		metaData.scriptClass->addInternalCall("Internal_GetComponent", &ScriptComponent::internal_getComponent);
		metaData.scriptClass->addInternalCall("Internal_GetComponents", &ScriptComponent::internal_getComponents);
		metaData.scriptClass->addInternalCall("Internal_RemoveComponent", &ScriptComponent::internal_removeComponent);
		metaData.scriptClass->addInternalCall("Internal_GetSceneObject", &ScriptComponent::internal_getSceneObject);
	}

	void ScriptComponent::setManagedComponent(const GameObjectHandle<ManagedComponent>& managedComponent)
	{
		mManagedComponent = managedComponent;
	}

	MonoObject* ScriptComponent::internal_addComponent(MonoObject* parentSceneObject, MonoReflectionType* type)
	{
		ScriptSceneObject* scriptSO = ScriptSceneObject::toNative(parentSceneObject);
		HSceneObject so = static_object_cast<SceneObject>(scriptSO->getNativeHandle());

		if (checkIfDestroyed(so))
			return nullptr;

		GameObjectHandle<ManagedComponent> mc = so->addComponent<ManagedComponent>(type);

		return mc->getManagedInstance();
	}

	MonoObject* ScriptComponent::internal_getComponent(MonoObject* parentSceneObject, MonoReflectionType* type)
	{
		ScriptSceneObject* scriptSO = ScriptSceneObject::toNative(parentSceneObject);
		HSceneObject so = static_object_cast<SceneObject>(scriptSO->getNativeHandle());

		if (checkIfDestroyed(so))
			return nullptr;

		const Vector<HComponent>& mComponents = so->getComponents();
		for(auto& component : mComponents)
		{
			if(component->getTypeId() == TID_ManagedComponent)
			{
				GameObjectHandle<ManagedComponent> managedComponent = static_object_cast<ManagedComponent>(component);

				if(managedComponent->getRuntimeType() == type)
				{
					return managedComponent->getManagedInstance();
				}
			}
		}

		return nullptr;
	}

	MonoArray* ScriptComponent::internal_getComponents(MonoObject* parentSceneObject)
	{
		ScriptSceneObject* scriptSO = ScriptSceneObject::toNative(parentSceneObject);
		HSceneObject so = static_object_cast<SceneObject>(scriptSO->getNativeHandle());

		Vector<MonoObject*> managedComponents;

		if (!checkIfDestroyed(so))
		{
			const Vector<HComponent>& mComponents = so->getComponents();
			for (auto& component : mComponents)
			{
				if (component->getTypeId() == TID_ManagedComponent)
				{
					GameObjectHandle<ManagedComponent> managedComponent = static_object_cast<ManagedComponent>(component);

					managedComponents.push_back(managedComponent->getManagedInstance());
				}
			}
		}

		MonoArray* componentArray = mono_array_new(MonoManager::instance().getDomain(), 
			metaData.scriptClass->_getInternalClass(), (UINT32)managedComponents.size());

		for(UINT32 i = 0; i < (UINT32)managedComponents.size(); i++)
		{
			void* elemAddr = mono_array_addr_with_size(componentArray, sizeof(MonoObject*), i);
			memcpy(elemAddr, &managedComponents[i], sizeof(MonoObject*));
		}

		return componentArray;
	}

	void ScriptComponent::internal_removeComponent(MonoObject* parentSceneObject, MonoReflectionType* type)
	{
		ScriptSceneObject* scriptSO = ScriptSceneObject::toNative(parentSceneObject);
		HSceneObject so = static_object_cast<SceneObject>(scriptSO->getNativeHandle());

		if (checkIfDestroyed(so))
			return;

		// We only allow single component per type
		const Vector<HComponent>& mComponents = so->getComponents();
		for(auto& component : mComponents)
		{
			if(component->getTypeId() == TID_ManagedComponent)
			{
				GameObjectHandle<ManagedComponent> managedComponent = static_object_cast<ManagedComponent>(component);

				if(managedComponent->getRuntimeType() == type)
				{
					managedComponent->destroy();
					return;
				}
			}
		}

		LOGWRN("Attempting to remove a component that doesn't exists on SceneObject \"" + so->getName() + "\"");
	}

	MonoObject* ScriptComponent::internal_getSceneObject(ScriptComponent* nativeInstance)
	{
		if (checkIfDestroyed(nativeInstance->mManagedComponent))
			return nullptr;

		HSceneObject sceneObject = nativeInstance->mManagedComponent->sceneObject();

		ScriptSceneObject* scriptSO = ScriptGameObjectManager::instance().getScriptSceneObject(sceneObject);
		if (scriptSO == nullptr)
			scriptSO = ScriptGameObjectManager::instance().createScriptSceneObject(sceneObject);

		return scriptSO->getManagedInstance();
	}

	bool ScriptComponent::checkIfDestroyed(const GameObjectHandleBase& handle)
	{
		if (handle.isDestroyed())
		{
			LOGWRN("Trying to access a destroyed GameObject with instance ID: " + handle.getInstanceId());
			return true;
		}

		return false;
	}

	MonoObject* ScriptComponent::_createManagedInstance(bool construct)
	{
		ManagedSerializableObjectInfoPtr currentObjInfo = nullptr;

		// See if this type even still exists
		if (!ScriptAssemblyManager::instance().getSerializableObjectInfo(mNamespace, mType, currentObjInfo))
		{
			mTypeMissing = true;
			return ScriptAssemblyManager::instance().getMissingComponentClass()->createInstance(true);
		}

		mTypeMissing = false;
		return currentObjInfo->mMonoClass->createInstance(construct);
	}

	ScriptObjectBackup ScriptComponent::beginRefresh()
	{
		ScriptGameObjectBase::beginRefresh();

		ScriptObjectBackup backupData;

		// It's possible that managed component is destroyed but a reference to it
		// is still kept. Don't backup such components.
		if (!mManagedComponent.isDestroyed(true))
			backupData.data = mManagedComponent->backup(true);

		return backupData;
	}

	void ScriptComponent::endRefresh(const ScriptObjectBackup& backupData)
	{
		ComponentBackupData componentBackup = any_cast<ComponentBackupData>(backupData.data);
		mManagedComponent->restore(mManagedInstance, componentBackup, mTypeMissing);
		
		ScriptGameObjectBase::endRefresh(backupData);
	}

	void ScriptComponent::_onManagedInstanceDeleted()
	{
		mManagedInstance = nullptr;

		// It's possible that managed component is destroyed but a reference to it
		// is still kept during assembly refresh. Such components shouldn't be restored
		// so we delete them.
		if (!mRefreshInProgress || mManagedComponent.isDestroyed(true))
			ScriptGameObjectManager::instance().destroyScriptGameObject(this);
	}

	void ScriptComponent::setNativeHandle(const HGameObject& gameObject)
	{
		mManagedComponent = static_object_cast<ManagedComponent>(gameObject);
	}
}