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
		 * @brief	Calculates the size of elements in a layout of the specified size.
		 * 
		 * @param	width				Width of the layout.
		 * @param	height				Height of the layout.
		 * @param	layout				Parent layout of the children to calculate the area for.
		 * @param	updateOptimalSizes	Optimization (doesn't change the results). Set to false if
		 *								GUIElementBase::_updateOptimalLayoutSizes was already called and optimal sizes are 
		 *								up to date to avoid recalculating them. (Normally that is true if this is being 
		 *								called during GUI layout update)
		 */
		static Vector2I calcActualSize(UINT32 width, UINT32 height, GUILayout* layout, bool updateOptimalSizes = true);

	private:
		/**
		 * @brief	Calculates the size of elements in a layout of the specified size. Assumes the layout and all its children
		 *			have updated optimal sizes.
		 * 
		 * @param	width				Width of the layout.
		 * @param	height				Height of the layout.
		 * @param	layout				Parent layout of the children to calculate the area for.
		 */
		static Vector2I calcActualSizeInternal(UINT32 width, UINT32 height, GUILayout* layout);
	};
}
