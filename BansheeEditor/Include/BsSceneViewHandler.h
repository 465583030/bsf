#pragma once

#include "BsEditorPrerequisites.h"
#include "BsVector2I.h"

namespace BansheeEngine
{
	/**
	 * @brief	Handles various operations specific to the scene view, like updating,
	 *			selecting and rendering handles, selecting objects, rendering scene grid, etc.
	 *
	 * @note	Internal class.
	 */
	class BS_ED_EXPORT SceneViewHandler
	{
	public:
		SceneViewHandler(EditorWidgetBase* parentWidget, const SPtr<Camera>& camera);
		virtual ~SceneViewHandler();

		/**
		 * @brief	Called once per frame. Updates gizmos and the scene grid.
		 */
		void update();

		/**
		 * @brief	Updates currently active handles.
		 *
		 * @param	position	Position of the pointer relative to the scene camera viewport.
		 * @param	delta		Movement of the pointer since last frame.
		 */
		void updateHandle(const Vector2I& position, const Vector2I& delta);

		/**
		 * @brief	Updates the selection overlay for currently selected object(s).
		 */
		void updateSelection();

		/**
		 * @brief	Selects a handle under the pointer position.
		 *
		 * @param	position	Position of the pointer relative to the scene camera viewport.
		 */
		void trySelectHandle(const Vector2I& position);

		/**
		 * @brief	Is any handle currently active.
		 */
		bool isHandleActive() const;

		/**
		 * @brief	Deselects any currently active handles.
		 */
		void clearHandleSelection();

		/**
		 * @brief	Attempts to select a scene object under the pointer position.
		 *
		 * @param	position	Position of the pointer relative to the scene camera viewport.
		 * @param	additive	Should this selection add to the existing selection, or replace it.
		 */
		void pickObject(const Vector2I& position, bool additive);

	protected:
		/**
		 * @brief	Checks is the pointer currently within the provided window, and if it is not
		 *			the cursor is wrapped in such a way so that it is returned to within the window bounds.
		 *		
		 * @return	How far was the cursor moved due to wrapping. This will be (0, 0) if the cursor is within
		 *			window bounds initially.
		 */
		Vector2I wrapCursorToWindow();

	private:
		EditorWidgetBase* mParentWidget;
		SPtr<Camera> mCamera;
		SceneGrid* mSceneGrid;
		SelectionRenderer* mSelectionRenderer;

		Vector2I mMouseDeltaCompensate;
	};
}