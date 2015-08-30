#pragma once

#include "BsEditorPrerequisites.h"
#include "BsEvent.h"

namespace BansheeEngine
{
	/**
	 * @brief	A GUI object that contains one or multiple editor widgets.
	 *			Each widget is represented by a single tab and can be activated,
	 *			deactivated, moved or dragged off.
	 *
	 * @note	Internal class.
	 */
	class BS_ED_EXPORT EditorWidgetContainer
	{
	public:
		EditorWidgetContainer(CGUIWidget* parent, EditorWindowBase* parentEditorWindow);
		virtual ~EditorWidgetContainer();

		/**
		 * @brief	Adds a new widget to the container, adding a new tab to
		 *			the end of the tab bar.
		 */
		void add(EditorWidgetBase& widget);

		/**
		 * @brief	Removes a widget from the container.
		 */
		void remove(EditorWidgetBase& widget);

		/**
		 * @brief	Inserts a widget at the specified index, adding a new
		 *			tab at that position.
		 *
		 * @param	idx		Sequential index where to insert the widget.
		 * @param	widget	Widget to insert.
		 */
		void insert(UINT32 idx, EditorWidgetBase& widget);

		/**
		 * @brief	Checks if the container already contains the provided widget.
		 */
		bool contains(EditorWidgetBase& widget);

		/**
		 * @brief	Sets the size of the container in pixels. This also modifies
		 *			the size of all child widgets.
		 */
		void setSize(UINT32 width, UINT32 height);

		/**
		 * @brief	Sets the position of the container, relative to the parent
		 *			GUI widget. This also modifies the position of all child widgets.
		 */
		void setPosition(INT32 x, INT32 y);

		/**
		 * @brief	Returns the number of widgets currently docked in this container.
		 */
		UINT32 getNumWidgets() const { return (UINT32)mWidgets.size(); }

		/**
		 * @brief	Returns a widget at the specified index.
		 *
		 * @param	idx		Sequential index of the widget to retrieve.
		 */
		EditorWidgetBase* getWidget(UINT32 idx) const;

		/**
		 * @brief	Returns a widget that is currently visible (its tab is active).
		 */
		EditorWidgetBase* getActiveWidget() const;

		/**
		 * @brief	Returns the parent GUI widget the container is using to render the GUI on.
		 */
		CGUIWidget& getParentWidget() const { return *mParent; }

		/**
		 * @brief	Returns the parent editor window the container is docked in.
		 */
		EditorWindowBase* getParentWindow() const { return mParentWindow; }

		/**
		 * @brief	Returns bounds not including the tabbed title bar. These are the bounds
		 *			available to child widget GUI.
		 */
		Rect2I getContentBounds() const;

		/**
		 * @brief	Returns a list of areas that can be dragged off. These are normally areas
		 *			represented by tab buttons.
		 */
		Vector<Rect2I> getDraggableAreas() const;

		/**
		 * @brief	Called once per frame. Calls update on all docked widgets.
		 */
		void update();

		/**
		 * @brief	Updates the tabbed title bar by refreshing the names of all docked widgets.
		 */
		void refreshWidgetNames();

		/**
		 * @brief	Triggers when a widget is about to be destroyed.
		 */
		void _notifyWidgetDestroyed(EditorWidgetBase* widget);

		Event<void()> onWidgetAdded; /**< Triggered whenever a new widget is added to this container. */
		Event<void()> onWidgetClosed; /**< Triggered whenever a widget docked in this container is closed. */
	private:
		EditorWindowBase* mParentWindow;
		GUITabbedTitleBar* mTitleBar;
		GUIPanel* mTitleBarPanel;
		CGUIWidget* mParent;
		INT32 mX, mY;
		UINT32 mWidth, mHeight;
		UnorderedMap<UINT32, EditorWidgetBase*> mWidgets;
		INT32 mActiveWidget;

		static const UINT32 TitleBarHeight;

		/**
		 * @brief	Removes a widget without triggering a widget closed event.
		 */
		void removeInternal(EditorWidgetBase& widget);

		/**
		 * @brief	Changes the currently active widget to the one at the specified index. Making
		 *			the widget active means it will be visible in the container.
		 *
		 * @param	idx		Unique widget index (not sequential).
		 */
		void setActiveWidget(UINT32 idx);

		/**
		 * @brief	Triggered when a user clicks on a tab in the tabbed title bar.
		 *
		 * @param	idx		Unique widget index (not sequential) of the tab that was activated.
		 */
		void tabActivated(UINT32 idx);

		/**
		 * @brief	Triggered when a user closes a tab in the tabbed title bar.
		 *
		 * @param	idx		Unique widget index (not sequential) of the tab that was closed.
		 */
		void tabClosed(UINT32 idx);

		/**
		 * @brief	Triggered when a user drags a tab off the tabbed title bar.
		 *
		 * @param	idx		Unique widget index (not sequential) of the tab that was dragged off.
		 */
		void tabDraggedOff(UINT32 idx);

		/**
		 * @brief	Triggered when a user drags a tab on the tabbed title bar.
		 *
		 * @param	idx		Sequential index at the position on which the tab has been dragged on.
		 */
		void tabDraggedOn(UINT32 idx);

		/**
		 * @brief	Triggered when the widget drag and drop operation finishes.
		 *
		 * @param	wasDragProcessed	Signals whether any object handled the drop.
		 */
		static void tabDroppedCallback(bool wasDragProcessed);
	};
}