//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsSceneEditorWidget.h"
#include "BsEditorWindow.h"
#include "BsGUILabel.h"
#include "BsGUIButton.h"
#include "BsGUIInputBox.h"
#include "BsGUIScrollArea.h"
#include "BsGUIArea.h"
#include "BsGUILayout.h"
#include "BsEditorWidgetManager.h"
#include "BsRenderTexture.h"
#include "BsSceneObject.h"
#include "BsSceneCameraController.h"
#include "BsCamera.h"
#include "BsGUIRenderTexture.h"
#include "BsCoreThread.h"
#include "BsEditorWidgetContainer.h"
#include "BsRendererManager.h"
#include "BsCoreRenderer.h"
#include "BsGUIWidget.h"
#include "BsSceneGrid.h"
#include "BsInput.h"
#include "BsGUILayoutUtility.h"
#include "BsScenePicking.h"
#include "BsHandleManager.h"
#include "BsSelection.h"
#include "BsEditorApplication.h"
#include "BsProjectSettings.h"
#include "BsGizmoManager.h"

// DEBUG ONLY
#include "BsTime.h"
#include "BsResources.h"
#include "BsPath.h"
#include "BsGUITexture.h"
#include "BsSpriteTexture.h"
#include "BsFileSystem.h"

using namespace std::placeholders;

namespace BansheeEngine
{
	SceneEditorWidget* SceneEditorWidget::Instance = nullptr;

	SceneEditorWidget::SceneEditorWidget(const ConstructPrivately& dummy, EditorWidgetContainer& parentContainer)
		:EditorWidget<SceneEditorWidget>(HString(L"SceneEditorWidget"), parentContainer), mGUIRenderTexture(nullptr), 
		mLeftButtonPressed(false)
	{
		updateRenderTexture(getWidth(), getHeight());

		mRenderCallback = RendererManager::instance().getActive()->onRenderViewport.connect(std::bind(&SceneEditorWidget::render, this, _1, _2));
		mSceneGrid = bs_new<SceneGrid>();

		mOnPointerMovedConn = gInput().onPointerMoved.connect(std::bind(&SceneEditorWidget::onPointerMoved, this, _1));
		mOnPointerPressedConn = gInput().onPointerPressed.connect(std::bind(&SceneEditorWidget::onPointerPressed, this, _1));
		mOnPointerReleasedConn = gInput().onPointerReleased.connect(std::bind(&SceneEditorWidget::onPointerReleased, this, _1));

		determineParentWindow();
	}

	SceneEditorWidget::~SceneEditorWidget()
	{
		bs_delete(mSceneGrid);
		mRenderCallback.disconnect();
		mOnPointerMovedConn.disconnect();
		mOnPointerPressedConn.disconnect();
		mOnPointerReleasedConn.disconnect();

		GizmoManager::instance().clearRenderData();
	}

	void SceneEditorWidget::_update()
	{
		ProjectSettingsPtr projSettings = gEditorApplication().getProjectSettings();

		if (mCameraController)
		{
			mCameraController->update();
		}

		CameraHandlerPtr camera = mCamera->_getHandler();

		GizmoManager::instance().update(camera);
		HandleManager& handleManager = HandleManager::instance();

		handleManager.setDefaultHandleSize(projSettings->getHandleSize());
		handleManager.setMoveHandleSnapAmount(projSettings->getMoveHandleSnap());
		handleManager.setRotateHandleSnapAmount(projSettings->getRotationHandleSnap());
		handleManager.setScaleHandleSnapAmount(projSettings->getScaleHandleSnap());
		handleManager.update(camera);

		mSceneGrid->setSize(projSettings->getGridSize());
		mSceneGrid->setSpacing(projSettings->getGridSpacing());
		mSceneGrid->setMajorAxisSpacing(projSettings->getGridMajorAxisSpacing());
		mSceneGrid->setAxisMarkerSpacing(projSettings->getGridAxisMarkerSpacing());

		//// DEBUG ONLY
		//if (gTime().getCurrentFrameNumber() == 100)
		//{
		//	HTexture colorTex = mSceneRenderTarget->getBindableColorTexture();
		//	gResources().save(colorTex, "C:\\SavedRenderTex.asset", true);

		//	FileSystem::move("C:\\SavedRenderTex.asset", "C:\\SavedRenderTexNew.asset", true);

		//	HTexture colorTexLoaded = gResources().load("C:\\SavedRenderTexNew.asset");
		//	HSpriteTexture spriteTex = SpriteTexture::create(colorTexLoaded);

		//	GUILayout& layout = mContent->getLayout();
		//	layout.addElement(GUITexture::create(spriteTex));
		//}
	}

	bool SceneEditorWidget::toSceneViewPos(const Vector2I& screenPos, Vector2I& scenePos)
	{
		Vector2I windowPos = mParentWindow->screenToWindowPos(screenPos);

		Rect2I renderTextureBounds = GUILayoutUtility::calcBounds(mGUIRenderTexture);

		if (renderTextureBounds.contains(windowPos))
		{
			scenePos.x = windowPos.x - renderTextureBounds.x;
			scenePos.y = windowPos.y - renderTextureBounds.y;

			return true;
		}

		return false;
	}

	void SceneEditorWidget::onPointerMoved(const PointerEvent& event)
	{
		Vector2I scenePos;
		if (!toSceneViewPos(event.screenPos, scenePos))
			return;

		CameraHandlerPtr camera = mCamera->_getHandler();

		HandleManager::instance().handleInput(camera, scenePos, mLeftButtonPressed);
	}

	void SceneEditorWidget::onPointerReleased(const PointerEvent& event)
	{
		if (event.button != PointerEventButton::Left)
			return;

		Vector2I scenePos;
		if (!toSceneViewPos(event.screenPos, scenePos))
			return;

		CameraHandlerPtr camera = mCamera->_getHandler();

		mLeftButtonPressed = false;
		HandleManager::instance().handleInput(camera, scenePos, mLeftButtonPressed);
	}

	void SceneEditorWidget::onPointerPressed(const PointerEvent& event)
	{
		if (event.button != PointerEventButton::Left)
			return;

		Vector2I scenePos;
		if (!toSceneViewPos(event.screenPos, scenePos))
			return;

		mLeftButtonPressed = true;

		CameraHandlerPtr camera = mCamera->_getHandler();

		// If we didn't hit a handle, perform normal selection
		if (!HandleManager::instance().hasHitHandle(camera, scenePos))
		{
			// TODO - Handle multi-selection (i.e. selection rectangle when dragging)
			HSceneObject pickedObject = ScenePicking::instance().pickClosestObject(camera, scenePos, Vector2I(1, 1));

			if (pickedObject)
			{
				if (event.control) // Append to existing selection
				{
					Vector<HSceneObject> selectedSOs = Selection::instance().getSceneObjects();

					auto iterFind = std::find_if(selectedSOs.begin(), selectedSOs.end(),
						[&](const HSceneObject& obj) { return obj == pickedObject; }
					);

					if (iterFind != selectedSOs.end())
						selectedSOs.push_back(pickedObject);

					Selection::instance().setSceneObjects(selectedSOs);
				}
				else
				{
					Vector<HSceneObject> selectedSOs = { pickedObject };

					Selection::instance().setSceneObjects(selectedSOs);
				}
			}
			else
				Selection::instance().clearSceneSelection();
		}

		// This also causes handles to be created/removed, so perform this only after
		// selection has been updated, otherwise we lag back a frame
		HandleManager::instance().handleInput(camera, scenePos, mLeftButtonPressed);
	}

	void SceneEditorWidget::doOnResized(UINT32 width, UINT32 height)
	{
		EditorWidget::doOnResized(width, height);

		updateRenderTexture(width, height);
	}

	void SceneEditorWidget::doOnParentChanged()
	{
		determineParentWindow();
	}

	void SceneEditorWidget::determineParentWindow()
	{
		mParentWindow = nullptr;

		if (mParent == nullptr)
			return;

		GUIWidget& parentWidget = getParentWidget();
		RenderTargetPtr parentTarget = parentWidget.getTarget()->getTarget();

		if (!parentTarget->getProperties().isWindow())
		{
			BS_EXCEPT(InvalidStateException, "Scene view parent render target is not a window. This is not supported.");
			return;
		}

		mParentWindow = std::static_pointer_cast<RenderWindow>(parentTarget);
	}

	void SceneEditorWidget::render(const Viewport* viewport, DrawList& drawList)
	{
		if (mCamera == nullptr)
			return;

		if (mCamera->getViewport().get() != viewport)
			return;

		mSceneGrid->render(mCamera.getInternalPtr(), drawList);
	}

	void SceneEditorWidget::updateRenderTexture(UINT32 width, UINT32 height)
	{
		width = std::max(20U, width);
		height = std::max(20U, height);

		if (mSceneRenderTarget != nullptr)
			mSceneRenderTarget->destroy();

		mSceneRenderTarget = RenderTexture::create(TEX_TYPE_2D, width, height);
		mSceneRenderTarget->setPriority(gCoreAccessor(), 1);

		if (mCamera == nullptr)
		{
			HSceneObject sceneCameraSO = SceneObject::create("SceneCamera");
			mCamera = sceneCameraSO->addComponent<Camera>(mSceneRenderTarget, 0.0f, 0.0f, 1.0f, 1.0f);

			sceneCameraSO->setPosition(Vector3(0, 0.5f, 1));
			sceneCameraSO->lookAt(Vector3(0, 0, 0));

			mCamera->setPriority(1);
			mCamera->setNearClipDistance(0.005f);
			mCamera->setFarClipDistance(1000.0f);

			mCameraController = sceneCameraSO->addComponent<SceneCameraController>();

			GUILayout& layout = mContent->getLayout();

			mGUIRenderTexture = GUIRenderTexture::create(mSceneRenderTarget);
			layout.addElement(mGUIRenderTexture);
		}
		else
		{
			mCamera->getViewport()->setTarget(mSceneRenderTarget);
			mGUIRenderTexture->setRenderTexture(mSceneRenderTarget);
		}

		// TODO - Consider only doing the resize once user stops resizing the widget in order to reduce constant
		// render target destroy/create cycle for every little pixel.

		mCamera->setAspectRatio(width / (float)height);
	}

	SceneEditorWidget* SceneEditorWidget::instance()
	{
		return Instance;
	}

	SceneEditorWidget* SceneEditorWidget::open()
	{
		return static_cast<SceneEditorWidget*>(EditorWidgetManager::instance().open(getTypeName()));
	}

	void SceneEditorWidget::close()
	{
		if (Instance != nullptr)
			EditorWidgetManager::instance().close(Instance);

		Instance = nullptr;
	}

	const String& SceneEditorWidget::getTypeName()
	{
		static String name = "SceneEditorWidget";
		return name;
	}
}