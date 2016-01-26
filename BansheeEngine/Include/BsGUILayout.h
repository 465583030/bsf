#pragma once

#include "BsPrerequisites.h"
#include "BsGUIElementBase.h"
#include "BsVector2I.h"

namespace BansheeEngine
{
	/**
	 * @brief	Base class for layout GUI element. Layout element positions and sizes
	 *			any child elements according to element styles and layout options.
	 */
	class BS_EXPORT GUILayout : public GUIElementBase
	{
	public:
		GUILayout();
		virtual ~GUILayout();

		/**
		 * @brief	Adds a new element to the layout after all existing elements.
		 */
		void addElement(GUIElement* element);

		/**
		 * @brief	Removes the specified element from the layout.
		 */
		void removeElement(GUIElement* element);

		/**
		 * @brief	Inserts a GUI element before the element at the specified index.
		 */
		void insertElement(UINT32 idx, GUIElement* element);

		/**
		 * @brief	Adds a new horizontal layout as a child of this layout.
		 */
		GUILayout& addLayoutX() { return addLayoutXInternal(this); }

		/**
		 * @brief	Adds a new vertical layout as a child of this layout.
		 */
		GUILayout& addLayoutY() { return addLayoutYInternal(this); }

		/**
		 * @brief	Removes the specified child layout.
		 */
		void removeLayout(GUILayout& layout) { removeLayoutInternal(layout); }

		/**
		 * @brief	Inserts a horizontal layout before the element at the specified index.
		 */
		GUILayout& insertLayoutX(UINT32 idx) { return insertLayoutXInternal(this, idx); }

		/**
		 * @brief	Inserts a vertical layout before the element at the specified index.
		 */
		GUILayout& insertLayoutY(UINT32 idx) { return insertLayoutYInternal(this, idx); }

		/**
		 * @brief	Adds a fixed space as a child of this layout. Size of space is specified in pixels.
		 */
		GUIFixedSpace& addSpace(UINT32 size);

		/**
		 * @brief	Removes an existing space from the layout.
		 */
		void removeSpace(GUIFixedSpace& space);

		/**
		 * @brief	Inserts a fixed space of "size" pixels before the element at the specified index.
		 */
		GUIFixedSpace& insertSpace(UINT32 idx, UINT32 size);

		/**
		 * @brief	Adds a flexible space as a child of this layout. Flexible space will grow/shrink
		 *			depending on elements around it, but it will always allow elements to keep their
		 *			optimal size.
		 */
		GUIFlexibleSpace& addFlexibleSpace();

		/**
		 * @brief	Removes an existing flexible space from the layout.
		 */
		void removeFlexibleSpace(GUIFlexibleSpace& space);

		/**
		 * @brief	Inserts a flexible space before an element at the specified index. Flexible space 
		 *			will grow/shrink depending on elements around it, but it will always allow elements 
		 *			to keep their optimal size.
		 */
		GUIFlexibleSpace& insertFlexibleSpace(UINT32 idx);

		/**
		 * @brief	Returns number of child elements in the layout.
		 */
		UINT32 getNumChildren() const { return (UINT32)mChildren.size(); }

		/**
		 * @brief	Removes a child element at the specified index.
		 */
		void removeChildAt(UINT32 idx);

		/**
		 * @copydoc	GUIElementBase::_getOptimalSize
		 */
		Vector2I _getOptimalSize() const { return Vector2I(mOptimalWidth, mOptimalHeight); }

		/**
		 * @copydoc	GUIElementBase::_getPadding
		 */
		const RectOffset& _getPadding() const;

		/**
		 * @copydoc	GUIElementBase::_getType
		 */
		Type _getType() const { return GUIElementBase::Type::Layout; }

		/**
		 * @brief	Gets an actual width (in pixels) of all the child elements in the layout.
		 *
		 * @note	updateLayoutInternal must be called whenever layout or its children change,
		 * 			otherwise this method will return an incorrect value.
		 * 			
		 *			Returned value is based on non-clipped element bounds.
		 */
		UINT32 _getActualWidth() const { return mActualWidth; }

		/**
		 * @brief	Gets an actual height (in pixels) of all the child elements in the layout.
		 *
		 * @note	updateLayoutInternal must be called whenever layout or its children change,
		 * 			otherwise this method will return an incorrect value.
		 * 			
		 *			Returned value is based on non-clipped element bounds.
		 */
		UINT32 _getActualHeight() const { return mActualHeight; }
	protected:
		Vector<Vector2I> mOptimalSizes;
		UINT32 mOptimalWidth;
		UINT32 mOptimalHeight;

		UINT32 mActualWidth;
		UINT32 mActualHeight;
	};
}