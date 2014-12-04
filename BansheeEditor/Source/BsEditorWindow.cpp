#include "BsEditorWindow.h"
#include "BsEditorWidgetContainer.h"
#include "BsEditorWindowManager.h"
#include "BsDragAndDropManager.h"
#include "BsRenderWindow.h"

namespace BansheeEngine
{
	EditorWindow::EditorWindow()
		:EditorWindowBase(), mWidgets(bs_new<EditorWidgetContainer>(mGUI.get(), this))
	{
		updateSize();
		
		mWidgets->onWidgetClosed.connect(std::bind(&EditorWindow::widgetRemoved, this));
	}

	EditorWindow::~EditorWindow()
	{
		bs_delete(mWidgets);
	}

	void EditorWindow::update()
	{
		mWidgets->update();
	}

	void EditorWindow::resized()
	{
		EditorWindowBase::resized();

		updateSize();
	}

	void EditorWindow::updateSize()
	{
		mWidgets->setPosition(1, 1);

		UINT32 widgetWidth = (UINT32)std::max(0, (INT32)getWidth() - 2);
		UINT32 widgetHeight = (UINT32)std::max(0, (INT32)getHeight() - 2);

		mWidgets->setSize(widgetWidth, widgetHeight);

		Platform::setCaptionNonClientAreas(*mRenderWindow->getCore().get(), mWidgets->getDraggableAreas());
	}

	void EditorWindow::widgetRemoved()
	{
		if(mWidgets->getNumWidgets() == 0)
		{
			// HACK - If widget is being handled by drag and drop we don't want to
			// destroy its parent window just yet because Windows doesn't approve of
			// windows being destroyed while mouse is being held down (some events won't get
			// fired). I should probably handle this at a lower level, in RenderWindowManager.
			if(DragAndDropManager::instance().isDragInProgress() && DragAndDropManager::instance().getDragTypeId() == (UINT32)DragAndDropType::EditorWidget)
			{
				hide();

				// Get notified when drag and drop is done
				DragAndDropManager::instance().addDropCallback(std::bind(&EditorWindow::closeWindowDelayed, this));
			}
			else
				close();
		}
	}

	void EditorWindow::closeWindowDelayed()
	{
		close();
	}

	EditorWindow* EditorWindow::create()
	{
		return EditorWindowManager::instance().create();
	}
}