#include "BsSceneGrid.h"
#include "BsMath.h"
#include "BsShapeMeshes3D.h"
#include "BsVertexDataDesc.h"
#include "BsMaterial.h"
#include "BsMesh.h"
#include "BsDrawList.h"
#include "BsBuiltinEditorResources.h"
#include "BsCamera.h"
#include "BsRect3.h"
#include "BsCoreThread.h"
#include "BsEditorSettings.h"
#include "BsRendererManager.h"
#include "BsRenderer.h"

namespace BansheeEngine
{
	const Color SceneGridCore::GRID_LINE_COLOR = Color(0.5f, 0.5f, 0.5f);
	const float SceneGridCore::LINE_WIDTH = 0.025f;
	const float SceneGridCore::LINE_BORDER_WIDTH = 0.00075f;
	const float SceneGridCore::FADE_OUT_START = 5.0f;
	const float SceneGridCore::FADE_OUT_END = 40.0f;

	SceneGrid::SceneGrid(const CameraHandlerPtr& camera)
		:mCoreDirty(true), mCore(nullptr)
	{
		mVertexDesc = bs_shared_ptr_new<VertexDataDesc>();
		mVertexDesc->addVertElem(VET_FLOAT3, VES_POSITION);
		mVertexDesc->addVertElem(VET_FLOAT3, VES_NORMAL);

		mCore.store(bs_new<SceneGridCore>(), std::memory_order_release);

		HMaterial gridMaterial = BuiltinEditorResources::instance().createSceneGridMaterial();
		SPtr<MaterialCore> materialCore = gridMaterial->getCore();
		gCoreAccessor().queueCommand(std::bind(&SceneGrid::initializeCore, this, camera->getCore(), materialCore));

		updateGridMesh();
	}

	SceneGrid::~SceneGrid()
	{
		gCoreAccessor().queueCommand(std::bind(&SceneGrid::destroyCore, this, mCore.load(std::memory_order_relaxed)));
	}

	void SceneGrid::initializeCore(const SPtr<CameraHandlerCore>& camera, const SPtr<MaterialCore>& material)
	{
		THROW_IF_NOT_CORE_THREAD;

		mCore.load()->initialize(camera, material);
	}

	void SceneGrid::destroyCore(SceneGridCore* core)
	{
		THROW_IF_NOT_CORE_THREAD;

		bs_delete(core);
	}

	void SceneGrid::setOrigin(const Vector3& origin)
	{
		if (mOrigin != origin)
		{
			mOrigin = origin;
			updateGridMesh();
		}
	}

	void SceneGrid::setSize(UINT32 size)
	{
		if (mSize != size)
		{
			mSize = size;
			updateGridMesh();
		}
	}

	void SceneGrid::setSpacing(float spacing)
	{
		if (mSpacing != spacing)
		{
			mSpacing = spacing;
			mCoreDirty = true;
		}
	}

	void SceneGrid::setSettings(const EditorSettingsPtr& settings)
	{
		mSettings = settings;
		updateFromEditorSettings();
	}

	void SceneGrid::update()
	{
		if (mSettings != nullptr && mSettingsHash != mSettings->getHash())
			updateFromEditorSettings();

		if (mCoreDirty)
		{
			SceneGridCore* core = mCore.load(std::memory_order_relaxed);
			gCoreAccessor().queueCommand(std::bind(&SceneGridCore::updateData, core, mGridMesh->getCore(), mSpacing));

			mCoreDirty = false;
		}
	}

	void SceneGrid::updateFromEditorSettings()
	{
		setSize(mSettings->getGridSize());
		setSpacing(mSettings->getGridSpacing());

		mSettingsHash = mSettings->getHash();
	}

	void SceneGrid::updateGridMesh()
	{
		std::array<Vector3, 2> axes;
		axes[0] = Vector3::UNIT_X;
		axes[1] = Vector3::UNIT_Z;

		std::array<float, 2> extents;
		extents[0] = mSize * 0.5f;
		extents[1] = mSize * 0.5f;

		Rect3 quad(mOrigin, axes, extents);
		MeshDataPtr meshData = bs_shared_ptr_new<MeshData>(8, 12, mVertexDesc);

		ShapeMeshes3D::solidQuad(quad, meshData, 0, 0);
		mGridMesh = Mesh::create(meshData);
		mCoreDirty = true;
	}

	SceneGridCore::~SceneGridCore()
	{
		CoreRendererPtr activeRenderer = RendererManager::instance().getActive();
		activeRenderer->_unregisterRenderCallback(mCamera.get(), -20);
	}

	void SceneGridCore::initialize(const SPtr<CameraHandlerCore>& camera, const SPtr<MaterialCore>& material)
	{
		mCamera = camera;
		mGridMaterial = material;

		mViewProjParam = mGridMaterial->getParamMat4("matViewProj");
		mWorldCameraPosParam = mGridMaterial->getParamVec4("worldCameraPos");
		mGridColorParam = mGridMaterial->getParamColor("gridColor");
		mGridSpacingParam = mGridMaterial->getParamFloat("gridSpacing");
		mGridBorderWidthParam = mGridMaterial->getParamFloat("gridBorderWidth");
		mGridFadeOutStartParam = mGridMaterial->getParamFloat("gridFadeOutStart");
		mGridFadeOutEndParam = mGridMaterial->getParamFloat("gridFadeOutEnd");

		CoreRendererPtr activeRenderer = RendererManager::instance().getActive();
		activeRenderer->_registerRenderCallback(camera.get(), -20, std::bind(&SceneGridCore::render, this));			
	}

	void SceneGridCore::updateData(const SPtr<MeshCore>& mesh, float spacing)
	{
		mGridMesh = mesh;
		mSpacing = spacing;
	}

	void SceneGridCore::render()
	{
		THROW_IF_NOT_CORE_THREAD;

		Matrix4 projMatrix = mCamera->getProjectionMatrixRS();
		Matrix4 viewMatrix = mCamera->getViewMatrix();

		Matrix4 viewProjMatrix = projMatrix * viewMatrix;
		mViewProjParam.set(viewProjMatrix);

		mWorldCameraPosParam.set(Vector4(mCamera->getPosition(), 1.0f));
		mGridColorParam.set(GRID_LINE_COLOR);
		mGridSpacingParam.set(mSpacing);
		mGridBorderWidthParam.set(LINE_BORDER_WIDTH);
		mGridFadeOutStartParam.set(FADE_OUT_START);
		mGridFadeOutEndParam.set(FADE_OUT_END);

		CoreRenderer::setPass(mGridMaterial, 0);
		CoreRenderer::draw(mGridMesh, mGridMesh->getProperties().getSubMesh(0));
	}
}