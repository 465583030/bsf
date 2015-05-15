#pragma once

#include "BsPrerequisites.h"
#include "BsVector2I.h"
#include "BsRect2I.h"

namespace BansheeEngine
{
	/**
	 * @brief	Determines how will the drop down box be positioned. Usually the system will attempt to position
	 * 			the drop box in a way so all elements can fit, and this class allows you to specify some limitations
	 * 			on how that works. 
	 * 			
	 * @note	For example, list boxes usually want drop down boxes to be placed above or below them, while
	 * 			context menus may want to have them placed around a single point in any direction.
	 */
	class BS_EXPORT DropDownAreaPlacement
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
		 * @brief	Preferred horizontal direction of the placement bounds, either
		 *			to the left or to the right of the wanted position/bounds.
		 */
		enum class HorzDir
		{
			Left, Right
		};

		/**
		 * @brief	Preferred horizontal direction of the placement bounds, either
		 *			upward or downward of the wanted position/bounds.
		 */
		enum class VertDir
		{
			Up, Down
		};

		/**
		 * @brief	Drop down box will be placed at the specified position. By default the system
		 * 			prefers the top left corner of the box to correspond to the position, but if
		 * 			other corners offer more space for the contents, those will be used instead.
		 */
		static DropDownAreaPlacement aroundPosition(const Vector2I& position);

		/**
		 * @brief	Drop down box will be placed at the specified bounds. Box will be horizontally aligned to the left
		 * 			of the provided bounds. Vertically system prefers placing the box at the bottom of the bounds, but may choose
		 * 			to align it with the top of the bounds if it offers more space for the contents.
		 */
		static DropDownAreaPlacement aroundBoundsVert(const Rect2I& bounds);
		
		/**
		 * @brief	Drop down box will be placed at the specified bounds. Box will be vertically aligned to the top
		 * 			of the provided bounds. Horizontally system prefers placing the box at the right of the bounds, but may choose
		 * 			to align it with the left of the bounds if it offers more space for the contents.
		 */
		static DropDownAreaPlacement aroundBoundsHorz(const Rect2I& bounds);

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

		/**
		 * @brief	Calculates the optimal bounds to place an element of the specified size, within
		 *			the available area using the internal data as a guide.
		 *
		 * @param	width	Width of the element to try to position, in pixels.
		 * @param	height	Height of the element to try to position, in pixels.
		 * @param	Rect2I	Available area to try to position the element in, in pixels.
		 * @param	horzDir	Output parameter that signals the preferred horizontal direction of the bounds (left or right).
		 * @param	vertDir	Output parameter that signals the preferred vertical direction of the bounds (up or down).
		 */
		Rect2I getOptimalBounds(UINT32 width, UINT32 height, const Rect2I& availableArea, HorzDir& horzDir, VertDir& vertDir) const;

	private:
		DropDownAreaPlacement() { }

		Type mType;
		Rect2I mBounds;
		Vector2I mPosition;
	};
}