#include "BsMainEditorWindow.h"
#include "BsEditorWindowManager.h"
#include "BsDockManager.h"
#include "BsGUIMenuBar.h"
#include "BsCamera.h"
#include "BsSceneObject.h"
#include "BsRenderTexture.h"
#include "BsApplication.h"
#include "BsProfilingManager.h"
#include "BsGUIArea.h"
#include "BsGUILayout.h"
#include "BsEditorTestSuite.h"
#include "BsTestOutput.h"

// DEBUG ONLY
#include "BsTestTextSprite.h"
#include "BsDebugCamera.h"
#include "BsDrawHelper2D.h"
#include "BsDrawHelper3D.h"
#include "BsRectF.h"
#include "BsProfilerOverlay.h"
#include "BsPlatform.h"

namespace BansheeEngine
{
	MainEditorWindow* MainEditorWindow::create(const RenderWindowPtr& renderWindow)
	{
		return EditorWindowManager::instance().createMain(renderWindow);
	}

	MainEditorWindow::MainEditorWindow(const RenderWindowPtr& renderWindow)
		:EditorWindowBase(renderWindow), mDockManager(nullptr), 
		mMenuBar(bs_new<GUIMenuBar>(mGUI.get(), mRenderWindow.get()))
	{
		mDockManager = DockManager::create(mRenderWindow.get());

		GUIArea* dockArea = GUIArea::createStretchedXY(*mGUI.get(), 0, 0, 0, 0);
		dockArea->getLayout().addElement(mDockManager);

		updateAreas();

		mMenuBar->addMenuItem(L"File/New project", nullptr);
		mMenuBar->addMenuItem(L"File/Open project", nullptr);
		mMenuBar->addSeparator(L"File");
		mMenuBar->addMenuItem(L"File/Recent projects/Project A", nullptr);
		mMenuBar->addMenuItem(L"File/Recent projects/Project B", nullptr);
		mMenuBar->addMenuItem(L"File/Recent projects/Project C", nullptr);
		mMenuBar->addSeparator(L"File");
		mMenuBar->addMenuItem(L"File/Exit", nullptr);
		mMenuBar->addMenuItem(L"Window/Scene", nullptr);

		// DEBUG ONLY

		RenderTexturePtr sceneRenderTarget = RenderTexture::create(TEX_TYPE_2D, 800, 600);
		sceneRenderTarget->setPriority(1);

		HSceneObject sceneCameraGO = SceneObject::create("SceneCamera");
		HCamera sceneCamera = sceneCameraGO->addComponent<Camera>(sceneRenderTarget, 0.0f, 0.0f, 1.0f, 1.0f);

		sceneCamera->setPriority(1);
		sceneCameraGO->setPosition(Vector3(0,50,1240));
		sceneCameraGO->lookAt(Vector3(0,50,-300));
		sceneCamera->setNearClipDistance(5);
		sceneCamera->setAspectRatio(800.0f / 600.0f);

		GameObjectHandle<DebugCamera> debugCamera = sceneCameraGO->addComponent<DebugCamera>();

		GameObjectHandle<TestTextSprite> textSprite = mSceneObject->addComponent<TestTextSprite>(mCamera->getViewport().get());

		textSprite->init(sceneCamera, "Testing in a new row, does this work?", sceneRenderTarget);

		//DrawHelper2D::instance().drawQuad(sceneCamera, FRect(0.0f, 0.2f, 0.75f, 0.5f), Color::White, DebugDrawCoordType::Normalized, 250.0f);
		//DrawHelper2D::instance().drawQuad(sceneCamera, FRect(50.0f, 50.0f, 100.0f, 50.0f), Color::Blue, DebugDrawCoordType::Pixel, 250.0f);

		//DrawHelper2D::instance().drawLine_Pixel(sceneCamera, Vector2(0, 0), Vector2(20, 20), Color::Blue, DebugDrawCoordType::Pixel, 250.0f);

		//DrawHelper2D::instance().drawLine_AA(sceneCamera, Vector2(100, 10), Vector2(120, 40), 1.0f, 1.0f, Color::Blue, DebugDrawCoordType::Pixel, 250.0f);

		AABox dbgBox(Vector3(-300, -200, 1000), Vector3(300, 300, 1500));
		//DrawHelper3D::instance().drawAABox(sceneCamera, dbgBox, Color::Green, 250.0f);

		mProfilerOverlay = mSceneObject->addComponent<ProfilerOverlay>(sceneCamera->getViewport());
		mProfilerOverlay->show(ProfilerOverlayType::CPUSamples);

		TestSuitePtr testSuite = TestSuite::create<EditorTestSuite>();
		testSuite->run(ExceptionTestOutput());
	}

	MainEditorWindow::~MainEditorWindow()
	{
		mDockManager->closeAll();
		GUIElement::destroy(mDockManager);
		bs_delete(mMenuBar);
	}

	void MainEditorWindow::resized()
	{
		EditorWindowBase::resized();

		updateAreas();
	}

	void MainEditorWindow::updateAreas()
	{
		UINT32 widgetWidth = (UINT32)std::max(0, (INT32)getWidth() - 2);
		UINT32 widgetHeight = (UINT32)std::max(0, (INT32)getHeight() - 2);

		UINT32 menuBarHeight = 15;
		mMenuBar->setArea(1, 1, widgetWidth, menuBarHeight);

		UINT32 dockHeight = (UINT32)std::max(0, (INT32)widgetHeight - (INT32)menuBarHeight);
		mDockManager->setArea(1, menuBarHeight + 1, widgetWidth, dockHeight);
	}

	void MainEditorWindow::update()
	{
		mDockManager->update();
	}
}