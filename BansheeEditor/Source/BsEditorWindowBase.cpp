#include "BsEditorWindowBase.h"
#include "BsCoreApplication.h"
#include "BsCoreThread.h"
#include "BsSceneObject.h"
#include "BsRenderWindow.h"

#include "BsEditorWindowManager.h"
#include "BsCamera.h"
#include "BsGUIWindowFrameWidget.h"
#include "BsBuiltinEditorResources.h"

namespace BansheeEngine
{
	EditorWindowBase::EditorWindowBase()
		:mOwnsRenderWindow(true)
	{
		RENDER_WINDOW_DESC renderWindowDesc;
		renderWindowDesc.videoMode = VideoMode(200, 200);
		renderWindowDesc.title = "EditorWindow";
		renderWindowDesc.fullscreen = false;
		renderWindowDesc.border = WindowBorder::None;
		renderWindowDesc.toolWindow = true;

		mRenderWindow = RenderWindow::create(renderWindowDesc, gCoreApplication().getPrimaryWindow());

		construct(mRenderWindow);
	}

	EditorWindowBase::EditorWindowBase(const RenderWindowPtr& renderWindow)
		:mOwnsRenderWindow(false)
	{
		construct(renderWindow);
	}

	EditorWindowBase::~EditorWindowBase()
	{
		mResizedConn.disconnect();

		if(mOwnsRenderWindow)
			mRenderWindow->destroy();

		mSceneObject->destroy();
	}

	void EditorWindowBase::initialize()
	{
		setPosition(0, 0);
		setSize(200, 200);
	}

	void EditorWindowBase::close()
	{
		EditorWindowManager::instance().destroy(this);
	}

	void EditorWindowBase::hide()
	{
		mRenderWindow->hide(gCoreAccessor());
	}

	void EditorWindowBase::construct(const RenderWindowPtr& renderWindow)
	{
		mRenderWindow = renderWindow;
		mSceneObject = SceneObject::create("EditorWindow");

		mCamera = mSceneObject->addComponent<Camera>(renderWindow, 0.0f, 0.0f, 1.0f, 1.0f);
		mCamera->setNearClipDistance(5);
		mCamera->setAspectRatio(1.0f);
		mCamera->setIgnoreSceneRenderables(true);

		mGUI = mSceneObject->addComponent<GUIWidget>(mCamera->getViewport().get());
		mGUI->setDepth(128);

		mGUI->setSkin(BuiltinEditorResources::instance().getSkin());

		mWindowFrame = mSceneObject->addComponent<WindowFrameWidget>(mCamera->getViewport().get(), renderWindow.get(), BuiltinEditorResources::instance().getSkin());
		mWindowFrame->setDepth(129);

		mResizedConn = renderWindow->onResized.connect(std::bind(&EditorWindowBase::resized, this));
	}

	void EditorWindowBase::setPosition(INT32 x, INT32 y)
	{
		mRenderWindow->move(gCoreAccessor(), x, y);
	}

	void EditorWindowBase::setSize(UINT32 width, UINT32 height)
	{
		mRenderWindow->resize(gCoreAccessor(), width, height);
	}

	INT32 EditorWindowBase::getLeft() const
	{
		return mRenderWindow->getProperties().getLeft();
	}

	INT32 EditorWindowBase::getTop() const
	{
		return mRenderWindow->getProperties().getTop();
	}

	UINT32 EditorWindowBase::getWidth() const
	{
		return (UINT32)mRenderWindow->getProperties().getWidth();
	}

	UINT32 EditorWindowBase::getHeight() const
	{
		return (UINT32)mRenderWindow->getProperties().getHeight();
	}
}