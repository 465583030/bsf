#pragma once

#include "BsEditorPrerequisites.h"
#include "BsEditorWidgetManager.h"
#include "BsEvent.h"
#include "BsRect2I.h"

namespace BansheeEngine
{
	/**
	 * @brief	Editor widget represents a single "window" in the editor. It may be dragged,
	 *			docked and can share space with multiple other widgets by using tabs.
	 *
	 * Each widget has its own position, size, GUI and update method.
	 * Widget is always docked inside an EditorWidgetContainer unless it's being dragged
	 * and is not visible.
	 */
	class BS_ED_EXPORT EditorWidgetBase
	{
	public:
		/**
		 * @brief	Gets a unique name for this widget. This name will be used for referencing
		 *			the widget by other systems.
		 */
		const String& getName() const { return mName; }

		/**
		 * @brief	Gets the display name for the widget. This is what editor users will see
		 *			in the widget title bar.
		 */
		const HString& getDisplayName() const { return mDisplayName; }

		/**
		 * @brief	Returns the X position of the widget inside of its container.
		 */
		INT32 getX() const { return mX; }

		/**
		 * @brief	Returns the Y position of the widget inside of its container.
		 */
		INT32 getY() const { return mY; }

		/**
		 * @brief	Returns the width of the widget in pixels.
		 */
		UINT32 getWidth() const { return mWidth; }

		/**
		 * @brief	Returns the height of the widget in pixels.
		 */
		UINT32 getHeight() const { return mHeight; }

		/**
		 * @brief	Returns the width of the widget when initially created, in pixels.
		 */
		UINT32 getDefaultWidth() const { return mDefaultWidth; }

		/**
		 * @brief	Returns the height of the widget when initially created, in pixels.
		 */
		UINT32 getDefaultHeight() const { return mDefaultHeight; }

		/**
		 * @brief	Returns the bounds of the widget in pixels, relative
		 *			to its parent container.
		 */
		Rect2I getBounds() const { return Rect2I(mX, mY, mWidth, mHeight); }

		/**
		 * @brief	Checks if the widget has focus (usually means user clicked on it last).
		 */
		bool hasFocus() const { return mHasFocus; }

		/**
		 * @brief	Gets the parent editor window this widget is docked in.
		 */
		EditorWindowBase* getParentWindow() const;

		/**
		 * @brief	Changes the size of the widget (and its internal GUI panel).
		 * 
		 * @note	Internal method.
		 */
		void _setSize(UINT32 width, UINT32 height);

		/**
		 * @brief	Changes the position of the widget (and its internal GUI panel), 
		 *			relative to its parent container.
		 * 
		 * @note	Internal method.
		 */
		void _setPosition(INT32 x, INT32 y);

		/**
		 * @brief	Changes the parent container of the widget (e.g. when re-docking or moving
		 *			a widget to another window). Parent can be null (e.g. when widget is in the
		 *			process of dragging and not visible).
		 *
		 * @note	Internal method.
		 */
		void _changeParent(EditorWidgetContainer* parent);

		/**
		 * @brief	Sets or removes focus for this widget.
		 *
		 * @note	Internal method.
		 */
		void _setHasFocus(bool focus);

		/**
		 * @brief	Returns the parent widget container. Can be null (e.g. when widget is in the
		 *			process of dragging and not visible).
		 *
		 * @note	Internal method.
		 */
		EditorWidgetContainer* _getParent() const { return mParent; }

		/**
		 * @brief	Converts screen coordinates to coordinates relative to the
		 *			widget.
		 */
		Vector2I screenToWidgetPos(const Vector2I& screenPos) const;

		/**
		 * @brief	Converts widget relative coordinates to screen coordiantes.
		 */
		Vector2I widgetToScreenPos(const Vector2I& widgetPos) const;

		/**
		 * @brief	Disables the widget making its GUI contents not visible. The widget
		 *			remains docked in its container.
		 *
		 * @note	Internal method.
		 */
		void _disable();

		/**
		 * @brief	Enables the widget making its previously hidden GUI contents visible.
		 *
		 * @note	Internal method.
		 */
		void _enable();

		/**
		 * @brief	Closes the widget, undocking it from its container and freeing any resources
		 *			related to it.
		 */
		void close();

		/**
		 * @brief	Called once per frame.
		 *
		 * @note	Internal method.
		 */
		virtual void update() { }

		Event<void(UINT32, UINT32)> onResized; /**< Triggered whenever widget size changes. */
		Event<void(INT32, INT32)> onMoved; /**< Triggered whenever widget position changes. */
		Event<void(EditorWidgetContainer*)> onParentChanged; /**< Triggered whenever widget parent container changes. */
		Event<void(bool)> onFocusChanged; /**< Triggered whenever widget receives or loses focus. */
	protected:
		friend class EditorWidgetManager;

		EditorWidgetBase(const HString& displayName, const String& name, UINT32 defaultWidth,
			UINT32 defaultHeight, EditorWidgetContainer& parentContainer);
		virtual ~EditorWidgetBase();

		/**
		 * @brief	Triggered whenever widget position changes.
		 */
		virtual void doOnMoved(INT32 x, INT32 y);

		/**
		 * @brief	Triggered whenever widget size changes.
		 */
		virtual void doOnResized(UINT32 width, UINT32 height);

		/**
		 * @brief	Triggered whenever widget parent container changes.
		 */
		virtual void doOnParentChanged();

		/**
		 * @brief	Returns the parent GUI widget. Before calling this you must ensure
		 *			the widget has a container parent otherwise this method will fail.
		 */
		CGUIWidget& getParentWidget() const;

		/**
		 * @brief	Frees widget resources and deletes the instance.
		 */
		static void destroy(EditorWidgetBase* widget);

		String mName;
		HString mDisplayName;
		EditorWidgetContainer* mParent;
		INT32 mX, mY;
		UINT32 mWidth, mHeight;
		UINT32 mDefaultWidth, mDefaultHeight;
		GUIPanel* mContent;
		bool mHasFocus;
	};

	/**
	 * @brief	Helper class that registers a widget creation callback with the widget manager.
	 *			The creation callback allows the runtime to open widgets just by their name
	 *			without knowing the actual type.
	 */
	template<typename Type>
	struct RegisterWidgetOnStart
	{
	public:
		RegisterWidgetOnStart()
		{
			EditorWidgetManager::preRegisterWidget(Type::getTypeName(), &create);
		}

		/**
		 * @brief	Creates a new widget of a specific type and adds it to the provided container.
		 */
		static EditorWidgetBase* create(EditorWidgetContainer& parentContainer)
		{
			return bs_new<Type>(EditorWidget<Type>::ConstructPrivately(), parentContainer);
		}
	};

	/**
	 * @brief	Editor widget template class that widgets can inherit from. Ensures that
	 *			all widget implementations are automatically registered with the widget manager.
	 *
	 * @see		EditorWidgetBase
	 */
	template <class Type>
	class EditorWidget : public EditorWidgetBase
	{
		static volatile RegisterWidgetOnStart<Type> RegisterOnStart;

	protected:
		friend struct RegisterWidgetOnStart<Type>;

		struct ConstructPrivately {};

		EditorWidget(const HString& displayName, UINT32 defaultWidth, UINT32 defaultHeight, 
			EditorWidgetContainer& parentContainer)
			:EditorWidgetBase(displayName, Type::getTypeName(), defaultWidth, defaultHeight, parentContainer)
		{ }

	public:

		virtual ~EditorWidget() { }
	};

	template <typename Type>
	volatile RegisterWidgetOnStart<Type> EditorWidget<Type>::RegisterOnStart;
}