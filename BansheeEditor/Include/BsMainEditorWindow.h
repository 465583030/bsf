#pragma once

#include "BsEditorPrerequisites.h"
#include "BsEditorWindowBase.h"

namespace BansheeEngine
{
	/**
	 * @brief	Primary editor window, containing the menu bar, status bar
	 *			and the dock area.
	 */
	class BS_ED_EXPORT MainEditorWindow : public EditorWindowBase
	{
	public:
		~MainEditorWindow();

		/**
		 * @copydoc	EditorWindowBase::update
		 */
		void update() override;

		/**
		 * @copydoc	EditorWindowBase::isMain
		 */
		virtual bool isMain() const override { return true; }

		/**
		 * @brief	Gets the DockManager that is responsible for docking and placement
		 *			of EditorWidget%s on the main window.
		 */
		DockManager& getDockManager() const { return *mDockManager; }

		/**
		 * @brief	Gets the primary menu bar element.
		 */
		GUIMenuBar& getMenuBar() const { return *mMenuBar; }

		/**
		 * @brief	Creates a new main editor window. If one is already open this method
		 *			will return the existing one.
		 *
		 * @param	renderWindow	Previously created render window to initialize the main
		 *							editor window in.
		 *
		 * @return	Instance of the main editor window.
		 */
		static MainEditorWindow* create(const RenderWindowPtr& renderWindow);
	protected:
		friend class EditorWindowManager;
		MainEditorWindow(const RenderWindowPtr& renderWindow);

	protected:
		GUIMenuBar* mMenuBar;
		DockManager* mDockManager;
		GUIStatusBar* mStatusBar;
		HProfilerOverlay mProfilerOverlay;

		/**
		 * @copydoc	EditorWindowBase::resized
		 */
		virtual void resized() override;

		/**
		 * @brief	Updates the placement of child GUI elements and their non-client
		 *			areas (used for OS move/resize operations). Should be called after
		 *			window size changes.
		 */
		void updateAreas();
	};
}