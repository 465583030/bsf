#pragma once

#include "BsPrerequisites.h"
#include "BsGUILayout.h"

namespace BansheeEngine
{
	/**
	 * @brief	Represents a horizontal layout that will layout out its child elements
	 *			left to right.
	 */
	class BS_EXPORT GUILayoutX : public GUILayout
	{
	public:
		GUILayoutX(const GUIDimensions& dimensions);
		GUILayoutX() {};
		~GUILayoutX() {};

		/**
		 * @brief	Calculate optimal sizes of all child layout elements.
		 */
		void _updateOptimalLayoutSizes() override;

		/**
		 * @copydoc	GUIElementBase::_calculateLayoutSizeRange
		 */
		virtual LayoutSizeRange _calculateLayoutSizeRange() const override;

		/**
		 * @copydoc	GUILayout::_getElementAreas
		 */
		void _getElementAreas(const Rect2I& layoutArea, Rect2I* elementAreas, UINT32 numElements,
			const Vector<LayoutSizeRange>& sizeRanges, const LayoutSizeRange& mySizeRange) const override;

		/**
		 * @brief	Creates a new horizontal layout.
		 */
		static GUILayoutX* create();

		/**
		 * @brief	Creates a new horizontal layout.
		 *
		 * @param	options		Options that allow you to control how is the element positioned and sized.
		 */
		static GUILayoutX* create(const GUIOptions& options);

	protected:
		/**
		 * @copydoc	GUIElementBase::_updateLayoutInternal
		 */
		void _updateLayoutInternal(const GUILayoutData& data) override;
	};
}