#pragma once

#include "BsPrerequisites.h"
#include "BsRect2I.h"
#include "BsVector2I.h"

namespace BansheeEngine
{
	/**
	 * Helper class that performs various operations related to
	 * GUILayout and GUI element sizing/placement.
	 */
	class BS_EXPORT GUILayoutUtility
	{
	public:
		/**
		 * Calculates optimal size of a GUI element.
		 */
		static Vector2I calcOptimalSize(const GUIElementBase* elem);

		/**
		 * Calculates position and size of a GUI element in its current layout.
		 * Returned position is relative to parent GUI panel.
		 *
		 * @param	elem		Element to calculate bounds for.
		 * @param	relativeTo	Parent panel of the provided element relative to which to return the
		 *						bounds. If null the bounds relative to the first parent panel are returned.
		 *						Behavior is undefined if provided panel is not a parent of the element.
		 */
		// TODO - This method might fail if element is part of a more complex hierarchy
		// other than just GUILayouts and base elements (e.g. a tree view) because for a lot
		// of such custom container elements like tree view don't have method for calculating 
		// element bounds implemented
		static Rect2I calcBounds(const GUIElementBase* elem, GUIPanel* relativeTo = nullptr);

		/**
		 * @brief	Calculates the actual size of the layout taken up by all of its elements.
		 *			
		 * @note	Actual size means the bounds might be smaller or larger than the layout area itself.
		 *			If larger that means certain portions of the child elements will be clipped, and if
		 *			smaller certain portions of the layout area will be empty.
		 */
		static Vector2I calcActualSize(UINT32 width, UINT32 height, const GUILayout* layout);
	};
}
