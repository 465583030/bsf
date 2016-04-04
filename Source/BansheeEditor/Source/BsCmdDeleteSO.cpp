//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsCmdDeleteSO.h"
#include "BsSceneObject.h"
#include "BsComponent.h"
#include "BsMemorySerializer.h"

namespace BansheeEngine
{
	CmdDeleteSO::CmdDeleteSO(const WString& description, const HSceneObject& sceneObject)
		:EditorCommand(description), mSceneObject(sceneObject), mSerializedObject(nullptr), 
		mSerializedObjectParentId(0), mSerializedObjectSize(0)
	{

	}

	CmdDeleteSO::~CmdDeleteSO()
	{
		mSceneObject = nullptr;
		clear();
	}

	void CmdDeleteSO::clear()
	{
		mSerializedObjectSize = 0;
		mSerializedObjectParentId = 0;

		if (mSerializedObject != nullptr)
		{
			bs_free(mSerializedObject);
			mSerializedObject = nullptr;
		}
	}

	void CmdDeleteSO::execute(const HSceneObject& sceneObject, const WString& description)
	{
		// Register command and commit it
		CmdDeleteSO* command = new (bs_alloc<CmdDeleteSO>()) CmdDeleteSO(description, sceneObject);
		UndoRedo::instance().registerCommand(command);
		command->commit();
	}

	void CmdDeleteSO::commit()
	{
		clear();

		if (mSceneObject == nullptr || mSceneObject.isDestroyed())
			return;

		recordSO(mSceneObject);
		mSceneObject->destroy();
	}

	void CmdDeleteSO::revert()
	{
		if (mSceneObject == nullptr)
			return;

		HSceneObject parent;
		if (mSerializedObjectParentId != 0)
			parent = GameObjectManager::instance().getObject(mSerializedObjectParentId);

		GameObjectManager::instance().setDeserializationMode(GODM_RestoreExternal | GODM_UseNewIds);

		// Object might still only be queued for destruction, but we need to fully destroy it since we're about to replace
		// the potentially only reference to the old object
		if (!mSceneObject.isDestroyed())
			mSceneObject->destroy(true);

		MemorySerializer serializer;
		SPtr<SceneObject> restored = std::static_pointer_cast<SceneObject>(serializer.decode(mSerializedObject, mSerializedObjectSize));

		CmdUtility::restoreIds(restored->getHandle(), mSceneObjectProxy);
		restored->setParent(parent);
	}

	void CmdDeleteSO::recordSO(const HSceneObject& sceneObject)
	{
		MemorySerializer serializer;
		mSerializedObject = serializer.encode(mSceneObject.get(), mSerializedObjectSize);

		HSceneObject parent = mSceneObject->getParent();
		if (parent != nullptr)
			mSerializedObjectParentId = parent->getInstanceId();

		mSceneObjectProxy = CmdUtility::createProxy(mSceneObject);
	}
}