#include "BsGUILayoutY.h"
#include "BsGUIElement.h"
#include "BsGUISpace.h"
#include "BsMath.h"
#include "BsVector2I.h"

namespace BansheeEngine
{
	GUILayoutY::GUILayoutY(GUIArea* parentArea)
		:GUILayout(parentArea)
	{ }

	GUILayoutY::GUILayoutY(const GUIDimensions& dimensions)
		: GUILayout(dimensions)
	{ }

	LayoutSizeRange GUILayoutY::_calculateLayoutSizeRange() const
	{
		if (mIsDisabled)
			return LayoutSizeRange();

		Vector2I optimalSize;
		for (auto& child : mChildren)
		{
			LayoutSizeRange sizeRange = child->_calculateLayoutSizeRange();
			
			if (child->_getType() == GUIElementBase::Type::FixedSpace)
				sizeRange.optimal.x = 0;

			UINT32 paddingX = child->_getPadding().left + child->_getPadding().right;
			UINT32 paddingY = child->_getPadding().top + child->_getPadding().bottom;

			optimalSize.y += sizeRange.optimal.y + paddingY;
			optimalSize.x = std::max((UINT32)optimalSize.x, sizeRange.optimal.x + paddingX);
		}

		return _getDimensions().calculateSizeRange(optimalSize);
	}

	void GUILayoutY::_updateOptimalLayoutSizes()
	{
		// Update all children first, otherwise we can't determine our own optimal size
		GUIElementBase::_updateOptimalLayoutSizes();

		if(mChildren.size() != mChildSizeRanges.size())
			mChildSizeRanges.resize(mChildren.size());

		Vector2I optimalSize;

		UINT32 childIdx = 0;
		for(auto& child : mChildren)
		{
			LayoutSizeRange& childSizeRange = mChildSizeRanges[childIdx];

			if(child->_getType() == GUIElementBase::Type::FixedSpace)
			{
				GUIFixedSpace* fixedSpace = static_cast<GUIFixedSpace*>(child);

				childSizeRange = fixedSpace->_calculateLayoutSizeRange();
				childSizeRange.optimal.x = 0;
			}
			else if(child->_getType() == GUIElementBase::Type::Element)
			{
				childSizeRange = child->_calculateLayoutSizeRange();
			}
			else if (child->_getType() == GUIElementBase::Type::Layout || child->_getType() == GUIElementBase::Type::Panel)
			{
				GUILayout* layout = static_cast<GUILayout*>(child);
				childSizeRange = layout->_getCachedSizeRange();
			}

			UINT32 paddingX = child->_getPadding().left + child->_getPadding().right;
			UINT32 paddingY = child->_getPadding().top + child->_getPadding().bottom;

			optimalSize.y += childSizeRange.optimal.y + paddingY;
			optimalSize.x = std::max((UINT32)optimalSize.x, childSizeRange.optimal.x + paddingX);

			childIdx++;
		}

		mSizeRange = _getDimensions().calculateSizeRange(optimalSize);
	}

	void GUILayoutY::_getElementAreas(INT32 x, INT32 y, UINT32 width, UINT32 height, Rect2I* elementAreas, UINT32 numElements, 
		const Vector<LayoutSizeRange>& sizeRanges, const LayoutSizeRange& mySizeRange) const
	{
		assert(mChildren.size() == numElements);

		UINT32 totalOptimalSize = mySizeRange.optimal.y;
		UINT32 totalNonClampedSize = 0;
		UINT32 numNonClampedElements = 0;
		UINT32 numFlexibleSpaces = 0;

		bool* processedElements = nullptr;
		float* elementScaleWeights = nullptr;

		if (mChildren.size() > 0)
		{
			processedElements = stackAllocN<bool>((UINT32)mChildren.size());
			memset(processedElements, 0, mChildren.size() * sizeof(bool));

			elementScaleWeights = stackAllocN<float>((UINT32)mChildren.size());
			memset(elementScaleWeights, 0, mChildren.size() * sizeof(float));
		}

		// Set initial sizes, count number of children per type and mark fixed elements as already processed
		UINT32 childIdx = 0;
		for (auto& child : mChildren)
		{
			elementAreas[childIdx].height = sizeRanges[childIdx].optimal.y;

			if (child->_getType() == GUIElementBase::Type::FixedSpace)
			{
				processedElements[childIdx] = true;
			}
			else if (child->_getType() == GUIElementBase::Type::FlexibleSpace)
			{
				numFlexibleSpaces++;
				numNonClampedElements++;
			}
			else
			{
				const GUIDimensions& dimensions = child->_getDimensions();

				if (dimensions.fixedHeight())
					processedElements[childIdx] = true;
				else
				{
					numNonClampedElements++;
					totalNonClampedSize += elementAreas[childIdx].height;
				}
			}

			childIdx++;
		}

		// If there is some room left, calculate flexible space sizes (since they will fill up all that extra room)
		if (height > totalOptimalSize)
		{
			UINT32 extraSize = height - totalOptimalSize;
			UINT32 remainingSize = extraSize;

			// Flexible spaces always expand to fill up all unused space
			if (numFlexibleSpaces > 0)
			{
				float avgSize = remainingSize / (float)numFlexibleSpaces;

				childIdx = 0;
				for (auto& child : mChildren)
				{
					if (processedElements[childIdx])
					{
						childIdx++;
						continue;
					}

					UINT32 extraHeight = std::min((UINT32)Math::ceilToInt(avgSize), remainingSize);
					UINT32 elementHeight = elementAreas[childIdx].height + extraHeight;

					// Clamp if needed
					if (child->_getType() == GUIElementBase::Type::FlexibleSpace)
					{
						processedElements[childIdx] = true;
						numNonClampedElements--;
						elementAreas[childIdx].height = elementHeight;

						remainingSize = (UINT32)std::max(0, (INT32)remainingSize - (INT32)extraHeight);
					}

					childIdx++;
				}

				totalOptimalSize = height;
			}
		}

		// Determine weight scale for every element. When scaling elements up/down they will be scaled based on this weight.
		// Weight is to ensure all elements are scaled fairly, so elements that are large will get effected more than smaller elements.
		childIdx = 0;
		float invOptimalSize = 1.0f / totalNonClampedSize;
		for (auto& child : mChildren)
		{
			if (processedElements[childIdx])
			{
				childIdx++;
				continue;
			}

			elementScaleWeights[childIdx] = invOptimalSize * elementAreas[childIdx].height;

			childIdx++;
		}

		// Our optimal size is larger than maximum allowed, so we need to reduce size of some elements
		if (totalOptimalSize > height)
		{
			UINT32 extraSize = totalOptimalSize - height;
			UINT32 remainingSize = extraSize;

			// Iterate until we reduce everything so it fits, while maintaining
			// equal average sizes using the weights we calculated earlier
			while (remainingSize > 0 && numNonClampedElements > 0)
			{
				UINT32 totalRemainingSize = remainingSize;

				childIdx = 0;
				for (auto& child : mChildren)
				{
					if (processedElements[childIdx])
					{
						childIdx++;
						continue;
					}

					float avgSize = totalRemainingSize * elementScaleWeights[childIdx];

					UINT32 extraHeight = std::min((UINT32)Math::ceilToInt(avgSize), remainingSize);
					UINT32 elementHeight = (UINT32)std::max(0, (INT32)elementAreas[childIdx].height - (INT32)extraHeight);

					// Clamp if needed
					switch (child->_getType())
					{
					case GUIElementBase::Type::FlexibleSpace:
						elementAreas[childIdx].height = 0;
						processedElements[childIdx] = true;
						numNonClampedElements--;
						break;
					case GUIElementBase::Type::Element:
					case GUIElementBase::Type::Layout:
					case GUIElementBase::Type::Panel:
					{
						const LayoutSizeRange& childSizeRange = sizeRanges[childIdx];

						if (elementHeight == 0)
						{
							processedElements[childIdx] = true;
							numNonClampedElements--;
						}
						else if (childSizeRange.min.y > 0 && (INT32)elementHeight < childSizeRange.min.y)
						{
							elementHeight = childSizeRange.min.y;

							processedElements[childIdx] = true;
							numNonClampedElements--;
						}

						extraHeight = elementAreas[childIdx].height - elementHeight;
						elementAreas[childIdx].height = elementHeight;
						remainingSize = (UINT32)std::max(0, (INT32)remainingSize - (INT32)extraHeight);
					}
						break;
					}

					childIdx++;
				}
			}
		}
		else // We are smaller than the allowed maximum, so try to expand some elements
		{
			UINT32 extraSize = height - totalOptimalSize;
			UINT32 remainingSize = extraSize;

			// Iterate until we reduce everything so it fits, while maintaining
			// equal average sizes using the weights we calculated earlier
			while (remainingSize > 0 && numNonClampedElements > 0)
			{
				UINT32 totalRemainingSize = remainingSize;

				childIdx = 0;
				for (auto& child : mChildren)
				{
					if (processedElements[childIdx])
					{
						childIdx++;
						continue;
					}

					float avgSize = totalRemainingSize * elementScaleWeights[childIdx];
					UINT32 extraHeight = std::min((UINT32)Math::ceilToInt(avgSize), remainingSize);
					UINT32 elementHeight = elementAreas[childIdx].height + extraHeight;

					// Clamp if needed
					if (child->_getType() == GUIElementBase::Type::Element || child->_getType() == GUIElementBase::Type::Layout)
					{
						const LayoutSizeRange& childSizeRange = sizeRanges[childIdx];

						if (elementHeight == 0)
						{
							processedElements[childIdx] = true;
							numNonClampedElements--;
						}
						else if (childSizeRange.max.y > 0 && (INT32)elementHeight > childSizeRange.max.y)
						{
							elementHeight = childSizeRange.max.y;

							processedElements[childIdx] = true;
							numNonClampedElements--;
						}

						extraHeight = elementHeight - elementAreas[childIdx].height;
						elementAreas[childIdx].height = elementHeight;
						remainingSize = (UINT32)std::max(0, (INT32)remainingSize - (INT32)extraHeight);
					}
					else if (child->_getType() == GUIElementBase::Type::FlexibleSpace)
					{
						processedElements[childIdx] = true;
						numNonClampedElements--;
					}

					childIdx++;
				}
			}
		}

		if (elementScaleWeights != nullptr)
			stackDeallocLast(elementScaleWeights);

		if (processedElements != nullptr)
			stackDeallocLast(processedElements);

		// Compute offsets and width
		UINT32 yOffset = 0;
		childIdx = 0;

		for (auto& child : mChildren)
		{
			UINT32 elemHeight = elementAreas[childIdx].height;
			yOffset += child->_getPadding().top;

			UINT32 elemWidth = (UINT32)sizeRanges[childIdx].optimal.x;
			const GUIDimensions& dimensions = child->_getDimensions();
			if (!dimensions.fixedWidth())
			{
				elemWidth = width;
				if (dimensions.minWidth > 0 && elemWidth < dimensions.minWidth)
					elemWidth = dimensions.minWidth;

				if (dimensions.maxWidth > 0 && elemWidth > dimensions.maxWidth)
					elemWidth = dimensions.maxWidth;
			}

			elementAreas[childIdx].width = elemWidth;

			if (child->_getType() == GUIElementBase::Type::Element)
			{
				GUIElement* element = static_cast<GUIElement*>(child);

				UINT32 xPadding = element->_getPadding().left + element->_getPadding().right;
				INT32 xOffset = Math::ceilToInt((INT32)(width - (INT32)(elemWidth + xPadding)) * 0.5f);
				xOffset = std::max(0, xOffset);

				elementAreas[childIdx].x = x + xOffset;
				elementAreas[childIdx].y = y + yOffset;
			}
			else if (child->_getType() == GUIElementBase::Type::Layout || child->_getType() == GUIElementBase::Type::Panel)
			{
				elementAreas[childIdx].x = x;
				elementAreas[childIdx].y = y + yOffset;
			}

			yOffset += elemHeight + child->_getPadding().bottom;
			childIdx++;
		}
	}

	void GUILayoutY::_updateLayoutInternal(INT32 x, INT32 y, UINT32 width, UINT32 height, Rect2I clipRect, UINT8 widgetDepth, UINT16 areaDepth)
	{
		UINT32 numElements = (UINT32)mChildren.size();
		Rect2I* elementAreas = nullptr;
		
		if (numElements > 0)
			elementAreas = stackConstructN<Rect2I>(numElements);

		_getElementAreas(x, y, width, height, elementAreas, numElements, mChildSizeRanges, mSizeRange);

		// Now that we have all the areas, actually assign them
		UINT32 childIdx = 0;
		Rect2I* actualSizes = elementAreas; // We re-use the same array

		for(auto& child : mChildren)
		{
			Rect2I childArea = elementAreas[childIdx];

			Vector2I offset(childArea.x, childArea.y);

			if(child->_getType() == GUIElementBase::Type::Element)
			{
				GUIElement* element = static_cast<GUIElement*>(child);

				element->_setPosition(offset);
				element->_setHeight(childArea.height);
				element->_setWidth(childArea.width);
				element->_setWidgetDepth(widgetDepth);
				element->_setAreaDepth(areaDepth);

				Rect2I elemClipRect(clipRect.x - offset.x, clipRect.y - offset.y, clipRect.width, clipRect.height);
				element->_setClipRect(elemClipRect);

				Rect2I newClipRect(offset.x, offset.y, childArea.width, childArea.height);
				newClipRect.clip(clipRect);
				element->_updateLayoutInternal(offset.x, offset.y, childArea.width, childArea.height, newClipRect, widgetDepth, areaDepth);

				actualSizes[childIdx].width = childArea.width + element->_getPadding().left + element->_getPadding().right;
			}
			else if (child->_getType() == GUIElementBase::Type::Layout || child->_getType() == GUIElementBase::Type::Panel)
			{
				GUILayout* layout = static_cast<GUILayout*>(child);

				Rect2I newClipRect(childArea.x, childArea.y, width, childArea.height);
				newClipRect.clip(clipRect);
				layout->_updateLayoutInternal(childArea.x, childArea.y, width, childArea.height, newClipRect, widgetDepth, areaDepth);

				actualSizes[childIdx].width = layout->_getActualWidth();
			}
			else
			{
				actualSizes[childIdx].width = childArea.width;
			}

			actualSizes[childIdx].height = childArea.height + child->_getPadding().top + child->_getPadding().bottom;
			childIdx++;
		}

		Vector2I actualSize = _calcActualSize(x, y, actualSizes, numElements);
		mActualWidth = (UINT32)actualSize.x;
		mActualHeight = (UINT32)actualSize.y;

		if (elementAreas != nullptr)
			stackDeallocLast(elementAreas);

		_markAsClean();
	}

	Vector2I GUILayoutY::_calcActualSize(INT32 x, INT32 y, Rect2I* elementAreas, UINT32 numElements) const
	{
		Vector2I actualArea;
		for (UINT32 i = 0; i < numElements; i++)
		{
			Rect2I childArea = elementAreas[i];

			actualArea.x = std::max(actualArea.x, childArea.width);
			actualArea.y += childArea.height;
		}

		return actualArea;
	}

	GUILayoutY* GUILayoutY::create()
	{
		return bs_new<GUILayoutY>();
	}

	GUILayoutY* GUILayoutY::create(const GUIOptions& options)
	{
		return bs_new<GUILayoutY>(GUIDimensions::create(options));
	}
}