#include "BsCoreSceneManager.h"
#include "BsSceneObject.h"
#include "BsComponent.h"
#include "BsGameObjectManager.h"

namespace BansheeEngine
{
	std::function<void()> SceneManagerFactory::mFactoryMethod;

	CoreSceneManager::CoreSceneManager()
	{
		mRootNode = SceneObject::createInternal("SceneRoot");
	}

	CoreSceneManager::~CoreSceneManager()
	{
		if (mRootNode != nullptr && !mRootNode.isDestroyed())
			mRootNode->destroy(true);
	}

	void CoreSceneManager::clearScene(bool forceAll)
	{
		UINT32 numChildren = mRootNode->getNumChildren();

		UINT32 curIdx = 0;
		for (UINT32 i = 0; i < numChildren; i++)
		{
			HSceneObject child = mRootNode->getChild(curIdx);

			if (forceAll || !child->hasFlag(SOF_Persistent))
				child->destroy();
			else
				curIdx++;
		}

		GameObjectManager::instance().destroyQueuedObjects();
	}

	void CoreSceneManager::_setRootNode(const HSceneObject& root)
	{
		if (root == nullptr)
			return;

		HSceneObject oldRoot = mRootNode;

		UINT32 numChildren = oldRoot->getNumChildren();
		// Make sure to keep persistent objects

		UINT32 curIdx = 0;
		for (UINT32 i = 0; i < numChildren; i++)
		{
			HSceneObject child = oldRoot->getChild(curIdx);

			if (child->hasFlag(SOF_Persistent))
				child->setParent(root, false);
		}

		mRootNode = root;
		mRootNode->_setParent(HSceneObject());

		oldRoot->destroy();
	}

	void CoreSceneManager::_update()
	{
		Stack<HSceneObject> todo;
		todo.push(mRootNode);

		while(!todo.empty())
		{
			HSceneObject currentGO = todo.top();
			todo.pop();

			if (!currentGO->getActive(true))
				continue;
			                  
			const Vector<HComponent>& components = currentGO->getComponents();

			for(auto iter = components.begin(); iter != components.end(); ++iter)
			{
				(*iter)->update();
			}

			for(UINT32 i = 0; i < currentGO->getNumChildren(); i++)
				todo.push(currentGO->getChild(i));
		}

		GameObjectManager::instance().destroyQueuedObjects();
	}

	void CoreSceneManager::registerNewSO(const HSceneObject& node) 
	{ 
		if(mRootNode)
			node->setParent(mRootNode);
	}

	CoreSceneManager& gCoreSceneManager()
	{
		return CoreSceneManager::instance();
	}

	void SceneManagerFactory::create()
	{
		if (mFactoryMethod != nullptr)
			mFactoryMethod();
		else
			BS_EXCEPT(InvalidStateException, "Failed to initialize scene manager because no valid factory method is set.");
	}
}