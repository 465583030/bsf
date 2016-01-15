#include "BsMainEditorWindow.h"
#include "BsEditorWindowManager.h"
#include "BsDockManager.h"
#include "BsGUIMenuBar.h"
#include "BsCGUIWidget.h"
#include "BsGUIPanel.h"
#include "BsGUIStatusBar.h"
#include "BsEditorTestSuite.h"
#include "BsTestOutput.h"

namespace BansheeEngine
{
	const UINT32 MainEditorWindow::MENU_BAR_HEIGHT = 50;
	const UINT32 MainEditorWindow::STATUS_BAR_HEIGHT = 21;

	MainEditorWindow* MainEditorWindow::create(const RenderWindowPtr& renderWindow)
	{
		return EditorWindowManager::instance().createMain(renderWindow);
	}

	MainEditorWindow::MainEditorWindow(const RenderWindowPtr& renderWindow)
		:EditorWindowBase(renderWindow), mDockManager(nullptr), 
		mMenuBar(bs_new<GUIMenuBar>(mGUI->_getInternal(), mRenderWindow.get()))
	{
		mDockManager = DockManager::create(this);
		mGUI->getPanel()->addElement(mDockManager);
		mStatusBar = GUIStatusBar::create();
		mGUI->getPanel()->addElement(mStatusBar);

		updateAreas();

		mMenuBar->addMenuItemSeparator(L"File", 10000);
		mMenuBar->addMenuItem(L"File/Exit", nullptr, 10000);

		TestSuitePtr testSuite = TestSuite::create<EditorTestSuite>();
		testSuite->run(ExceptionTestOutput());
	}

	MainEditorWindow::~MainEditorWindow()
	{
		mDockManager->closeAll();
		GUIElement::destroy(mDockManager);
		GUIElement::destroy(mStatusBar);
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

		mMenuBar->setArea(1, 1, widgetWidth, MENU_BAR_HEIGHT);

		UINT32 dockHeight = (UINT32)std::max(0, (INT32)widgetHeight - (INT32)(MENU_BAR_HEIGHT + STATUS_BAR_HEIGHT));
		mDockManager->setArea(1, MENU_BAR_HEIGHT + 1, widgetWidth, dockHeight);

		mStatusBar->setPosition(1, 1 + MENU_BAR_HEIGHT + dockHeight);
		mStatusBar->setWidth(widgetWidth);
		mStatusBar->setHeight(STATUS_BAR_HEIGHT);
	}

	void MainEditorWindow::update()
	{
		mDockManager->update();
	}
}