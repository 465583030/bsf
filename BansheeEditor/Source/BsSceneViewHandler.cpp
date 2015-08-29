#include "BsSceneViewHandler.h"
#include "BsRendererManager.h"
#include "BsRenderer.h"
#include "BsGizmoManager.h"
#include "BsHandleManager.h"
#include "BsSceneGrid.h"
#include "BsSelection.h"
#include "BsScenePicking.h"
#include "BsCamera.h"
#include "BsEditorApplication.h"
#include "BsEditorWidget.h"
#include "BsEditorWindowBase.h"
#include "BsRenderWindow.h"
#include "BsCursor.h"
#include "BsSelectionRenderer.h"

#include "BsDebug.h"

using namespace std::placeholders;

namespace BansheeEngine
{
	SceneViewHandler::SceneViewHandler(EditorWidgetBase* parentWidget, const SPtr<Camera>& camera)
		:mCamera(camera), mSceneGrid(nullptr), mParentWidget(parentWidget)
	{
		mSelectionRenderer = bs_new<SelectionRenderer>();
		mSceneGrid = bs_new<SceneGrid>(mCamera);
		mSceneGrid->setSettings(gEditorApplication().getEditorSettings());
		HandleManager::instance().setSettings(gEditorApplication().getEditorSettings());
	}

	SceneViewHandler::~SceneViewHandler()
	{
		bs_delete(mSceneGrid);
		bs_delete(mSelectionRenderer);

		if (GizmoManager::isStarted()) // If not active, we don't care
			GizmoManager::instance().clearRenderData();
	}

	void SceneViewHandler::update()
	{
		GizmoManager::instance().update(mCamera);
		mSceneGrid->update();
	}

	void SceneViewHandler::updateHandle(const Vector2I& position, const Vector2I& delta)
	{
		// If mouse wrapped around last frame then we need to compensate for the jump amount
		Vector2I realDelta = delta - mMouseDeltaCompensate;
		mMouseDeltaCompensate = Vector2I::ZERO;

		if (HandleManager::instance().isHandleActive())
			mMouseDeltaCompensate = wrapCursorToWindow();

		HandleManager::instance().update(mCamera, position, realDelta);
	}

	void SceneViewHandler::updateSelection()
	{
		// Call this after handle update to ensure its drawn at the right place
		mSelectionRenderer->update(mCamera);
	}

	void SceneViewHandler::trySelectHandle(const Vector2I& position)
	{
		HandleManager::instance().trySelect(mCamera, position);
	}

	bool SceneViewHandler::isHandleActive() const
	{
		return HandleManager::instance().isHandleActive();
	}

	void SceneViewHandler::clearHandleSelection()
	{
		HandleManager::instance().clearSelection();
	}

	void SceneViewHandler::pickObject(const Vector2I& position, bool additive)
	{
		// TODO - Handle multi-selection (i.e. selection rectangle when dragging)
		HSceneObject pickedObject = ScenePicking::instance().pickClosestObject(mCamera, position, Vector2I(1, 1));

		if (pickedObject)
		{
			if (additive) // Append to existing selection
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

	Vector2I SceneViewHandler::wrapCursorToWindow()
	{
		if (mParentWidget == nullptr)
			return Vector2I();

		RenderWindowPtr parentWindow = mParentWidget->getParentWindow()->getRenderWindow();

		Vector2I windowPos = parentWindow->screenToWindowPos(Cursor::instance().getScreenPosition());
		const RenderWindowProperties& rwProps = parentWindow->getProperties();

		INT32 maxWidth = std::max(0, (INT32)rwProps.getWidth() - 1);
		INT32 maxHeight = std::max(0, (INT32)rwProps.getHeight() - 1);

		Vector2I offset;
		if (windowPos.x <= 0)
			offset.x = maxWidth;
		else if (windowPos.x >= maxWidth)
			offset.x = -maxWidth;

		if (windowPos.y <= 0)
			offset.y = maxHeight;
		else if (windowPos.y >= maxHeight)
			offset.y = -maxHeight;

		windowPos += offset;

		Vector2I wrappedScreenPos = parentWindow->windowToScreenPos(windowPos);
		Cursor::instance().setScreenPosition(wrappedScreenPos);

		return offset;
	}
}