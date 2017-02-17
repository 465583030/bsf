//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsModule.h"
#include "BsGameObject.h"

namespace bs
{
	/** @addtogroup Scene-Internal
	 *  @{
	 */

	/**	Contains information about a camera managed by the scene manager. */
	struct SceneCameraData
	{
		SceneCameraData() { }

		SceneCameraData(const SPtr<Camera>& camera, const HSceneObject& sceneObject)
			:camera(camera), sceneObject(sceneObject)
		{ }

		SPtr<Camera> camera;
		HSceneObject sceneObject;
	};

	/**	Contains information about a renderable managed by the scene manager. */
	struct SceneRenderableData
	{
		SceneRenderableData() { }

		SceneRenderableData(const SPtr<Renderable>& renderable, const HSceneObject& sceneObject)
			:renderable(renderable), sceneObject(sceneObject)
		{ }

		SPtr<Renderable> renderable;
		HSceneObject sceneObject;
	};

	/**	Contains information about a light managed by the scene manager. */
	struct SceneLightData
	{
		SceneLightData() { }

		SceneLightData(const SPtr<Light>& light, const HSceneObject& sceneObject)
			:light(light), sceneObject(sceneObject)
		{ }

		SPtr<Light> light;
		HSceneObject sceneObject;
	};

	/** Manages active SceneObjects and provides ways for querying and updating them or their components. */
	class BS_CORE_EXPORT SceneManager : public Module<SceneManager>
	{
	public:
		SceneManager();
		~SceneManager();

		/**	Returns the root scene object. */
		HSceneObject getRootNode() const { return mRootNode; }

		/**
		 * Destroys all scene objects in the scene.
		 *
		 * @param[in]	forceAll	If true, then even the persistent objects will be unloaded.
		 */
		void clearScene(bool forceAll = false);

		/** Returns all cameras in the scene. */
		const Map<Camera*, SceneCameraData>& getAllCameras() const { return mCameras; }

		/**
		 * Returns the camera in the scene marked as main. Main camera controls the final render surface that is displayed
		 * to the user. If there are multiple main cameras, the first one found returned.
		 */
		SceneCameraData getMainCamera() const;

		/**
		 * Sets the render target that the main camera in the scene (if any) will render its view to. This generally means
		 * the main game window when running standalone, or the Game viewport when running in editor.
		 */
		void setMainRenderTarget(const SPtr<RenderTarget>& rt);

		/**	Returns all renderables in the scene. */
		const Map<Renderable*, SceneRenderableData>& getAllRenderables() const { return mRenderables; }

		/** Notifies the scene manager that a new renderable was created. */
		void _registerRenderable(const SPtr<Renderable>& renderable, const HSceneObject& so);

		/**	Notifies the scene manager that a renderable was removed. */
		void _unregisterRenderable(const SPtr<Renderable>& renderable);

		/**	Notifies the scene manager that a new light was created. */
		void _registerLight(const SPtr<Light>& light, const HSceneObject& so);

		/**	Notifies the scene manager that a light was removed. */
		void _unregisterLight(const SPtr<Light>& light);

		/**	Notifies the scene manager that a new camera was created. */
		void _registerCamera(const SPtr<Camera>& camera, const HSceneObject& so);

		/**	Notifies the scene manager that a camera was removed. */
		void _unregisterCamera(const SPtr<Camera>& camera);

		/**	Notifies the scene manager that a camera either became the main camera, or has stopped being main camera. */
		void _notifyMainCameraStateChanged(const SPtr<Camera>& camera);

		/** Changes the root scene object. Any persistent objects will remain in the scene, now parented to the new root. */
		void _setRootNode(const HSceneObject& root);

		/** Called every frame. Calls update methods on all scene objects and their components. */
		void _update();

		/** Updates dirty transforms on any core objects that may be tied with scene objects. */
		void _updateCoreObjectTransforms();

	protected:
		friend class SceneObject;

		/**
		 * Register a new node in the scene manager, on the top-most level of the hierarchy.
		 * 			
		 * @param[in]	node	Node you wish to add. It's your responsibility not to add duplicate or null nodes. This 
		 *						method won't check.
		 *
		 * @note	
		 * After you add a node in the scene manager, it takes ownership of its memory and is responsible for releasing it.
		 * Do NOT add nodes that have already been added (if you just want to change their parent). Normally this 
		 * method will only be called by SceneObject.
		 */
		void registerNewSO(const HSceneObject& node);

		/**	Callback that is triggered when the main render target size is changed. */
		void onMainRenderTargetResized();

	protected:
		HSceneObject mRootNode;

		Map<Camera*, SceneCameraData> mCameras;
		Vector<SceneCameraData> mMainCameras;

		Map<Renderable*, SceneRenderableData> mRenderables;
		Map<Light*, SceneLightData> mLights;

		SPtr<RenderTarget> mMainRT;
		HEvent mMainRTResizedConn;
	};

	/**	Provides easy access to the SceneManager. */
	BS_CORE_EXPORT SceneManager& gSceneManager();

	/** @} */
}