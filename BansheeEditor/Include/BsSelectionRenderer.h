//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsEditorPrerequisites.h"
#include "BsColor.h"
#include "BsMatrix4.h"
#include "BsGpuParam.h"

namespace BansheeEngine
{
	/** @cond INTERNAL */
	/** @addtogroup Scene-Editor
	 *  @{
	 */

	class SelectionRendererCore;

	/**	Handles rendering of the selected SceneObject%s overlay. */
	class BS_ED_EXPORT SelectionRenderer
	{
		/**	Contains data about a selected mesh. */
		struct ObjectData
		{
			SPtr<MeshCoreBase> mesh;
			Matrix4 worldTfrm;
		};

	public:
		SelectionRenderer();
		~SelectionRenderer();

		/** Called once per frame. Updates the overlay depending on current selection. */
		void update(const CameraPtr& camera);

	private:
		friend class SelectionRendererCore;

		/**
		 * Initializes the core thread counterpart of the selection renderer.
		 *
		 * @param[in]	mat		Material used for selection rendering.
		 */
		void initializeCore(const SPtr<MaterialCore>& mat);

		/**
		 * Destroys the core thread counterpart of the selection renderer.
		 *
		 * @param[in]	core	Previously constructed core thread selection renderer instance.
		 */
		void destroyCore(SelectionRendererCore* core);

		std::atomic<SelectionRendererCore*> mCore;
	};

	/** Core thread version of the selection renderer, that handles actual rendering. */
	class SelectionRendererCore
	{
		friend class SelectionRenderer;
		
		struct PrivatelyConstuct { };

	public:
		SelectionRendererCore(const PrivatelyConstuct& dummy);
		~SelectionRendererCore();

	private:
		/**
		 * Initializes the selection renderer. Should be called right after construction.
		 *
		 * @param[in]	mat	Material used for selection rendering.
		 */
		void initialize(const SPtr<MaterialCore>& mat);

		/** Triggered by the Renderer when the overlay should be rendered. */
		void render();

		/**
		 * Updates the internal data that determines what will be rendered on the next render() call.
		 *
		 * @param[in]	camera		Camera to render the selection overlay in.
		 * @param[in]	objects		A set of objects to render with the selection overlay.
		 */
		void updateData(const SPtr<CameraCore>& camera, const Vector<SelectionRenderer::ObjectData>& objects);

		Vector<SelectionRenderer::ObjectData> mObjects;
		SPtr<CameraCore> mCamera;

		// Immutable
		SPtr<MaterialCore> mMaterial;
		GpuParamMat4Core mMatWorldViewProj;
		GpuParamColorCore mColor;

		static const Color SELECTION_COLOR;
	};

	/** @} */
	/** @endcond */
}