#include "BsGUILayoutUtility.h"
#include "BsGUIElementBase.h"
#include "BsGUILayout.h"
#include "BsGUIArea.h"
#include "BsGUIElement.h"
#include "BsGUIElementStyle.h"

namespace BansheeEngine
{
	Vector2I GUILayoutUtility::calcOptimalSize(const GUIElementBase* elem)
	{
		return elem->_calculateOptimalLayoutSize();
	}

	Rect2I GUILayoutUtility::calcBounds(const GUIElementBase* elem)
	{
		Rect2I parentArea;

		GUIElementBase* parent = elem->_getParent();
		if (parent != nullptr)
			parentArea = calcBounds(parent);
		else
		{
			assert(elem->_getType() == GUIElementBase::Type::Layout);
			const GUILayout* layout = static_cast<const GUILayout*>(elem);

			GUIArea* parentGUIArea = layout->_getParentGUIArea();
			parentArea.x = 0; // Zero because we want position relative to parent GUIArea
			parentArea.y = 0;
			parentArea.width = parentGUIArea->width();
			parentArea.height = parentGUIArea->height();

			return parentArea;
		}

		UINT32 numElements = (UINT32)parent->_getNumChildren();
		UINT32 myIndex = 0;

		Vector<Vector2I> optimalSizes;
		for (UINT32 i = 0; i < numElements; i++)
		{
			GUIElementBase* child = parent->_getChild(i);

			if (child == elem)
				myIndex = i;

			optimalSizes.push_back(calcOptimalSize(child));
		}

		Rect2I* elementAreas = nullptr;

		if (numElements > 0)
			elementAreas = stackConstructN<Rect2I>(numElements);

		parent->_getElementAreas(parentArea.x, parentArea.y, parentArea.width, parentArea.height, elementAreas, numElements, optimalSizes);
		Rect2I myArea = elementAreas[myIndex];

		if (elementAreas != nullptr)
			stackDeallocLast(elementAreas);

		return myArea;
	}

	Vector2I GUILayoutUtility::calcActualSize(UINT32 width, UINT32 height, const GUILayout* layout)
	{
		UINT32 numElements = (UINT32)layout->_getNumChildren();

		Vector<Vector2I> optimalSizes;
		for (UINT32 i = 0; i < numElements; i++)
		{
			GUIElementBase* child = layout->_getChild(i);
			optimalSizes.push_back(GUILayoutUtility::calcOptimalSize(child));
		}

		Rect2I* elementAreas = nullptr;

		if (numElements > 0)
			elementAreas = stackConstructN<Rect2I>(numElements);

		layout->_getElementAreas(0, 0, width, height, elementAreas, numElements, optimalSizes);
		Rect2I* actualAreas = elementAreas; // We re-use the same array

		for (UINT32 i = 0; i < numElements; i++)
		{
			GUIElementBase* child = layout->_getChild(i);
			Rect2I childArea = elementAreas[i];

			if (child->_getType() == GUIElementBase::Type::Layout)
			{
				Vector2I childActualSize = calcActualSize(childArea.width, childArea.height, static_cast<GUILayout*>(child));
				actualAreas[i].width = (UINT32)childActualSize.x;
				actualAreas[i].height = (UINT32)childActualSize.y;
			}
			else if (child->_getType() == GUIElementBase::Type::Element)
			{
				RectOffset padding = child->_getPadding();

				actualAreas[i].width = elementAreas[i].width + padding.left + padding.right;
				actualAreas[i].height = elementAreas[i].height + padding.top + padding.bottom;
			}
			else
			{
				actualAreas[i].width = elementAreas[i].width;
				actualAreas[i].height = elementAreas[i].height;
			}
		}

		Vector2I actualSize = layout->_calcActualSize(actualAreas, numElements);

		if (elementAreas != nullptr)
			stackDeallocLast(elementAreas);

		return actualSize;
	}
}