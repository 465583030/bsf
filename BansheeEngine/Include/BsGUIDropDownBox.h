#pragma once

#include "BsPrerequisites.h"
#include "BsGUIWidget.h"
#include "BsVector2I.h"
#include "BsEvent.h"

namespace BansheeEngine
{
	/**
	 * @brief	Contains items used for initializing one level in a drop down box hierarchy.
	 */
	struct BS_EXPORT GUIDropDownData
	{
		Vector<GUIDropDownDataEntry> entries;
		UnorderedMap<WString, HString> localizedNames;
	};

	/**
	 * @brief	Represents a single entry in a drop down box.
	 */
	class BS_EXPORT GUIDropDownDataEntry
	{
		enum class Type
		{
			Separator,
			Entry,
			SubMenu
		};

	public:
		/**
		 * @brief	Creates a new separator entry.
		 */
		static GUIDropDownDataEntry separator();

		/**
		 * @brief	Creates a new button entry with the specified callback that is triggered
		 *			when button is selected.
		 */
		static GUIDropDownDataEntry button(const WString& label, std::function<void()> callback, const WString& shortcutTag = StringUtil::WBLANK);

		/**
		 * @brief	Creates a new sub-menu entry that will open the provided drop down data
		 *			sub-menu when activated.
		 */
		static GUIDropDownDataEntry subMenu(const WString& label, const GUIDropDownData& data);

		/**
		 * @brief	Check is the entry a separator.
		 */
		bool isSeparator() const { return mType == Type::Separator; }

		/**
		 * @brief	Check is the entry a sub menu.
		 */
		bool isSubMenu() const { return mType == Type::SubMenu; }

		/**
		 * @brief	Returns display label of the entry (if an entry is a button or a sub-menu).
		 */
		const WString& getLabel() const { return mLabel; }

		/**
		 * @brief	Returns the shortcut key combination string that is to be displayed along the entry label.
		 */
		const WString& getShortcutTag() const { return mShortcutTag; }

		/**
		 * @brief	Returns a button callback if the entry (if an entry is a button).
		 */
		std::function<void()> getCallback() const { return mCallback; }

		/**
		 * @brief	Returns sub-menu data that is used for creating a sub-menu (if an entry is a sub-menu).
		 */
		const GUIDropDownData& getSubMenuData() const { return mChildData; }
	private:
		GUIDropDownDataEntry() { }

		std::function<void()> mCallback;
		GUIDropDownData mChildData;
		WString mLabel;
		WString mShortcutTag;
		Type mType; 
	};

	/**
	 * @brief	Determines how will the drop down box be positioned. Usually the system will attempt to position
	 * 			the drop box in a way so all elements can fit, and this class allows you to specify some limitations
	 * 			on how that works. 
	 * 			
	 * @note	For example, list boxes usually want drop down boxes to be placed above or below them, while
	 * 			context menus may want to have them placed around a single point in any direction.
	 */
	class BS_EXPORT GUIDropDownAreaPlacement
	{
	public:
		/**
		 * @brief	Determines how will the drop down box be positioned.
		 */
		enum class Type
		{
			Position,
			BoundsVert,
			BoundsHorz
		};

		/**
		 * @brief	Drop down box will be placed at the specified position. By default the system
		 * 			prefers the top left corner of the box to correspond to the position, but if
		 * 			other corners offer more space for the contents, those will be used instead.
		 */
		static GUIDropDownAreaPlacement aroundPosition(const Vector2I& position);

		/**
		 * @brief	Drop down box will be placed at the specified bounds. Box will be horizontally aligned to the left
		 * 			of the provided bounds. Vertically system prefers placing the box at the bottom of the bounds, but may choose
		 * 			to align it with the top of the bounds if it offers more space for the contents.
		 */
		static GUIDropDownAreaPlacement aroundBoundsVert(const Rect2I& bounds);
		
		/**
		 * @brief	Drop down box will be placed at the specified bounds. Box will be vertically aligned to the top
		 * 			of the provided bounds. Horizontally system prefers placing the box at the right of the bounds, but may choose
		 * 			to align it with the left of the bounds if it offers more space for the contents.
		 */
		static GUIDropDownAreaPlacement aroundBoundsHorz(const Rect2I& bounds);

		/**
		 * @brief	Returns drop down box positioning type.
		 */
		Type getType() const { return mType; }

		/**
		 * @brief	Returns bounds around which to position the drop down box
		 *			if one of the bounds positioning types is used.
		 */
		const Rect2I& getBounds() const { return mBounds; }

		/**
		 * @brief	Returns position around which to position the drop down box
		 *			if position positioning type is used.
		 */
		const Vector2I& getPosition() const { return mPosition; }

	private:
		GUIDropDownAreaPlacement() { }

		Type mType;
		Rect2I mBounds;
		Vector2I mPosition;
	};

	/**
	 * @brief	Type of drop down box types.
	 */
	enum class GUIDropDownType
	{
		ListBox,
		ContextMenu,
		MenuBar
	};

	/**
	 * @brief	This is a generic GUI drop down box class that can be used for:
	 * 			list boxes, menu bars or context menus.
	 */
	class BS_EXPORT GUIDropDownBox : public GUIWidget
	{
	public:
		/**
		 * @brief	Creates a new drop down box widget.
		 *
		 * @param	parent			Parent scene object to attach the drop down box to.
		 * @param	target			Viewport on which to open the drop down box.
		 * @param	placement		Determines how is the drop down box positioned in the visible area.
		 * @param	dropDownData	Data to use for initializing menu items of the drop down box.
		 * @param	skin			Skin to use for drop down box GUI elements.
		 * @param	type			Specific type of drop down box to display.
		 */
		GUIDropDownBox(const HSceneObject& parent, Viewport* target, const GUIDropDownAreaPlacement& placement,
			const GUIDropDownData& dropDownData, const HGUISkin& skin, GUIDropDownType type);
		~GUIDropDownBox();

	private:
		/**
		 * @brief	Contains data about a single drop down box sub-menu
		 */
		struct DropDownSubMenu
		{
			/**
			 * @brief	Represents a single sub-menu page.
			 */
			struct PageInfo
			{
				UINT32 idx;
				UINT32 start;
				UINT32 end;
				UINT32 height;
			};

		public:
			/**
			 * @brief	Creates a new drop down box sub-menu
			 *
			 * @param	owner			Owner drop down box this sub menu belongs to.
			 * @param	parent			Parent sub-menu. Can be null.
			 * @param	placement		Determines how is the sub-menu positioned in the visible area.
			 * @param	availableBounds	Available bounds (in pixels) in which the sub-menu may be opened.
			 * @param	dropDownData	Data to use for initializing menu items of the sub-menu.
			 * @param	skin			Skin to use for sub-menu GUI elements.
			 * @param	depthOffset		How much to offset the sub-menu depth. We want deeper levels of the
			 *							sub-menu hierarchy to be in front of lower levels, so you should
			 *							increase this value for each level of the sub-menu hierarchy.
			 */
			DropDownSubMenu(GUIDropDownBox* owner, DropDownSubMenu* parent, const GUIDropDownAreaPlacement& placement, 
				const Rect2I& availableBounds, const GUIDropDownData& dropDownData, GUIDropDownType type, UINT32 depthOffset);
			~DropDownSubMenu();

			/**
			 * @brief	Recreates all internal GUI elements for the entries of the current sub-menu page.
			 */
			void updateGUIElements();

			/**
			 * @brief	Moves the sub-menu to the previous page and displays its elements, if available.
			 */
			void scrollDown();

			/**
			 * @brief	Moves the sub-menu to the next page and displays its elements, if available.
			 */
			void scrollUp();

			/**
			 * @brief	Moves the sub-menu to the first page and displays its elements.
			 */
			void scrollToTop();

			/**
			 * @brief	Moves the sub-menu to the last page and displays its elements.
			 */
			void scrollToBottom();

			/**
			 * @brief	Calculates ranges for all the pages of the sub-menu.
			 */
			Vector<PageInfo> getPageInfos() const;

			/**
			 * @brief	Called when the user activates an element with the specified index.
			 *
			 * @param	bounds	Bounds of the GUI element that is used as a visual representation
			 *					of this drop down element.
			 */
			void elementActivated(UINT32 idx, const Rect2I& bounds);

			/**
			 * @brief	Called when the user selects an element with the specified index.
			 * 
			 * @param	idx		Index of the element that was selected.
			 */
			void elementSelected(UINT32 idx);

			/**
			 * @brief	Called when the user wants to close the currently open sub-menu.
			 */
			void closeSubMenu();

			/**
			 * @brief	Closes this sub-menu.
			 */
			void close();

			/**
			 * @brief	Returns actual visible bounds of the sub-menu.
			 */
			Rect2I getVisibleBounds() const { return mVisibleBounds; }

			/**
			 * @brief	Returns the drop box object that owns this sub-menu.
			 */
			GUIDropDownBox* getOwner() const { return mOwner; }

		public:
			GUIDropDownBox* mOwner;

			GUIDropDownType mType;
			GUIDropDownData mData;
			UINT32 mPage;
			INT32 x, y;
			UINT32 width, height;
			Rect2I mVisibleBounds;
			Rect2I mAvailableBounds;
			UINT32 mDepthOffset;
			bool mOpenedUpward;

			GUIButton* mScrollUpBtn;
			GUIButton* mScrollDownBtn;
			GUIDropDownContent* mContent;
			GUITexture* mBackgroundFrame;

			GUIPanel* mBackgroundPanel;
			GUIPanel* mContentPanel;
			GUILayout* mContentLayout;

			DropDownSubMenu* mParent;
			DropDownSubMenu* mSubMenu;
		};

	private:
		friend class GUIDropDownContent;

		/**
		 * @brief	Called when the specified sub-menu is opened.
		 */
		void notifySubMenuOpened(DropDownSubMenu* subMenu);

		/**
		 * @brief	Called when the specified sub-menu is opened.
		 */
		void notifySubMenuClosed(DropDownSubMenu* subMenu);

		/**
		 * @brief	Called when the drop down box loses focus (and should be closed).
		 */
		void dropDownFocusLost();

		/**
		 * @copydoc	GUIWidget::onDestroyed
		 */
		void onDestroyed() override;

	private:
		static const UINT32 DROP_DOWN_BOX_WIDTH;

		String mScrollUpStyle;
		String mScrollDownStyle;
		String mBackgroundStyle;
		String mContentStyle;
		HSpriteTexture mScrollUpBtnArrow;
		HSpriteTexture mScrollDownBtnArrow;

		DropDownSubMenu* mRootMenu;
		GUIDropDownHitBox* mHitBox;
		// Captures mouse clicks so that we don't trigger elements outside the drop down box when we just want to close it.
		// (Particular example is clicking on the button that opened the drop down box in the first place. Clicking will cause
		// the drop down to lose focus and close, but if the button still processes the mouse click it will be immediately opened again)
		GUIDropDownHitBox* mCaptureHitBox; 
	};
}