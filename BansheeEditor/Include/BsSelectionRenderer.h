#pragma once

#include "BsEditorPrerequisites.h"
#include "BsColor.h"
#include "BsMatrix4.h"
#include "BsGpuParam.h"

namespace BansheeEngine
{
	class SelectionRendererCore;

	/**
	 * @brief	Handles rendering of the selected SceneObject%s overlay.
	 */
	class SelectionRenderer
	{
		/**
		 * @brief	Contains data about a selected mesh.
		 */
		struct ObjectData
		{
			SPtr<MeshCoreBase> mesh;
			Matrix4 worldTfrm;
		};

	public:
		SelectionRenderer();
		~SelectionRenderer();

		/**
		 * @brief	Called once per frame. Updates the overlay depending on current selection.
		 *
		 * @note	Internal method.
		 */
		void update(const CameraHandlerPtr& camera);

	private:
		friend class SelectionRendererCore;

		/**
		 * @brief	Initializes the core thread counterpart of the selection renderer.
		 *
		 * @param	mat	Material used for selection rendering.
		 */
		void initializeCore(const SPtr<MaterialCore>& mat);

		/**
		 * @brief	Destroys the core thread counterpart of the selection renderer.
		 *
		 * @param	core	Previously constructed core thread selection renderer instance.
		 */
		void destroyCore(SelectionRendererCore* core);

		SelectionRendererCore* mCore;
	};

	/**
	 * @brief	Core thread version of the selection renderer, that handles
	 *			actual rendering.
	 */
	class SelectionRendererCore
	{
		friend class SelectionRenderer;
		
		struct PrivatelyConstuct { };

	public:
		SelectionRendererCore(const PrivatelyConstuct& dummy);

	private:
		/**
		 * @brief	Initializes the selection renderer. Should be called right
		 *			after construction.
		 *
		 * @param	mat	Material used for selection rendering.
		 */
		void initialize(const SPtr<MaterialCore>& mat);

		/**
		 * @brief	Triggered by the Renderer when the overlay should be rendered.
		 */
		void render();

		/**
		 * @brief	Updates the internal data that determines what will be rendered on the next
		 *			::render call.
		 *
		 * @param	camera	Camera to render the selection overlay in.
		 * @param	objects	A set of objects to render with the selection overlay.
		 */
		void updateData(const SPtr<CameraHandlerCore>& camera, const Vector<SelectionRenderer::ObjectData>& objects);

		Vector<SelectionRenderer::ObjectData> mObjects;
		SPtr<CameraHandlerCore> mCamera;

		// Immutable
		SPtr<MaterialCore> mMaterial;
		GpuParamMat4Core mMatWorldViewProj;
		GpuParamColorCore mColor;

		static const Color SELECTION_COLOR;
	};
}