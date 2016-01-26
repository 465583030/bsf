#pragma once

#include "BsPrerequisites.h"
#include "BsCoreSceneManager.h"

namespace BansheeEngine
{
	/**
	 * @brief	Manages active SceneObjects and provides ways for querying
	 *			and updating them or their components.
	 */
	class BS_EXPORT SceneManager : public CoreSceneManager
	{
	public:
		SceneManager() {}
		virtual ~SceneManager() {}

		/**
		 * @brief	Returns all cameras in the scene.
		 */
		virtual const Vector<HCamera>& getAllCameras() const = 0;

		/**
		 * @brief	Returns all renderables in the scene.
		 */
		virtual const Vector<HRenderable>& getAllRenderables() const = 0;

		/**
		 * @brief	Updates dirty transforms on any scene objects with a Renderable component.
		 */
		virtual void updateRenderableTransforms() = 0;

		/**
		 * @brief	Triggered whenever a renderable is removed from a SceneObject.
		 */
		Event<void(const HRenderable&)> onRenderableRemoved;

		/**
		 * @brief	Triggered whenever a camera is removed from a SceneObject.
		 */
		Event<void(const HCamera&)> onCameraRemoved;
	};

	/**
	 * @copydoc	SceneManager
	 */
	BS_EXPORT SceneManager& gBsSceneManager();
}