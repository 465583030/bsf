#include "BsSceneManager.h"
#include "BsSceneObject.h"
#include "BsRenderable.h"
#include "BsCamera.h"
#include "BsLight.h"
#include "BsDebug.h"

namespace BansheeEngine
{
	volatile SceneManager::InitOnStart SceneManager::DoInitOnStart;

	void SceneManager::_registerRenderable(const SPtr<Renderable>& renderable, const HSceneObject& so)
	{
		mRenderables[renderable.get()] = SceneRenderableData(renderable, so);
	}

	void SceneManager::_unregisterRenderable(const SPtr<Renderable>& renderable)
	{
		mRenderables.erase(renderable.get());
	}

	void SceneManager::_registerCamera(const SPtr<Camera>& camera, const HSceneObject& so)
	{
		mCameras[camera.get()] = SceneCameraData(camera, so);
	}

	void SceneManager::_unregisterCamera(const SPtr<Camera>& camera)
	{
		mCameras.erase(camera.get());
	}

	void SceneManager::_registerLight(const SPtr<Light>& light, const HSceneObject& so)
	{
		mLights[light.get()] = SceneLightData(light, so);
	}

	void SceneManager::_unregisterLight(const SPtr<Light>& light)
	{
		mLights.erase(light.get());
	}

	void SceneManager::_updateCoreObjectTransforms()
	{
		for (auto& renderablePair : mRenderables)
		{
			RenderablePtr handler = renderablePair.second.renderable;
			HSceneObject so = renderablePair.second.sceneObject;

			UINT32 curHash = so->getTransformHash();
			if (curHash != handler->_getLastModifiedHash())
			{
				Matrix4 transformNoScale = Matrix4::TRS(so->getWorldPosition(), so->getWorldRotation(), Vector3::ONE);

				handler->setTransform(so->getWorldTfrm(), transformNoScale);
				handler->_setLastModifiedHash(curHash);
			}

			if (so->getActive() != handler->getIsActive())
			{
				handler->setIsActive(so->getActive());
			}
		}

		for (auto& cameraPair : mCameras)
		{
			CameraPtr handler = cameraPair.second.camera;
			HSceneObject so = cameraPair.second.sceneObject;

			UINT32 curHash = so->getTransformHash();
			if (curHash != handler->_getLastModifiedHash())
			{
				handler->setPosition(so->getWorldPosition());
				handler->setRotation(so->getWorldRotation());

				handler->_setLastModifiedHash(curHash);
			}
		}

		for (auto& lightPair : mLights)
		{
			SPtr<Light> handler = lightPair.second.light;
			HSceneObject so = lightPair.second.sceneObject;

			UINT32 curHash = so->getTransformHash();
			if (curHash != handler->_getLastModifiedHash())
			{
				handler->setPosition(so->getWorldPosition());
				handler->setRotation(so->getWorldRotation());

				handler->_setLastModifiedHash(curHash);
			}
		}
	}

	SceneManager& SceneManager::instance()
	{
		return static_cast<SceneManager&>(CoreSceneManager::instance());
	}

	SceneManager* SceneManager::instancePtr()
	{
		return static_cast<SceneManager*>(CoreSceneManager::instancePtr());
	}

	SceneManager& gSceneManager()
	{
		return static_cast<SceneManager&>(gCoreSceneManager());
	}
}
