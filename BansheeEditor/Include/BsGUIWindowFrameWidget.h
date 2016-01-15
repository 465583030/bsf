#pragma once

#include "BsEditorPrerequisites.h"
#include "BsCGUIWidget.h"
#include "BsEvent.h"

namespace BansheeEngine
{
	/**
	 * A GUIWidget specialization that when attached to a window will create a window border, window background and provide 
	 * resize functionality.
	 */
	class WindowFrameWidget : public CGUIWidget
	{
	public:
		/**
		 * Constructs a new window frame.
		 *
		 * @param[in]	parent			Parent SceneObject to attach the Component to.
		 * @param[in]	allowResize		Should the widget set up resize handles that can be dragged by the user.
		 * @param[in]	camera			Camera to draw the GUI elements in.
		 * @param[in]	ownerWindow		Window that the frame widget will act on.
		 * @param[in]	skin			GUI skin used for the GUI child elements.
		 */
		WindowFrameWidget(const HSceneObject& parent, bool allowResize, const CameraPtr& camera, RenderWindow* ownerWindow, 
			const HGUISkin& skin);
		virtual ~WindowFrameWidget();

	protected:
		/** @copydoc GUIWidget::ownerWindowFocusChanged */
		void ownerWindowFocusChanged() override;

		/** @copydoc GUIWidget::ownerTargetResized */
		void ownerTargetResized() override;

		/**
		 * Updates the non-client areas that notify the OS where the interactable elements used for window resize 
		 * operations are. This should be called after any resize operations.
		 */
		void refreshNonClientAreas() const;

		static const UINT32 RESIZE_BORDER_WIDTH;

		bool mAllowResize;
		GUIPanel* mWindowFramePanel;
		RenderWindow* mParentWindow;
		GUIWindowFrame* mWindowFrame;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class CWindowFrameWidgetRTTI;
		static RTTITypeBase* getRTTIStatic();
		virtual RTTITypeBase* getRTTI() const override;

		WindowFrameWidget() { } // Serialization only
	};
}