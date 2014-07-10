//__________________________ Banshee Project - A modern game development toolkit _________________________________//
//_____________________________________ www.banshee-project.com __________________________________________________//
//________________________ Copyright (c) 2014 Marko Pintera. All rights reserved. ________________________________//
#include "BsGUILayoutY.h"
#include "BsGUIElement.h"
#include "BsGUISpace.h"
#include "BsMath.h"
#include "BsVector2I.h"

namespace BansheeEngine
{
	void GUILayoutY::_updateOptimalLayoutSizes()
	{
		// Update all children first, otherwise we can't determine out own optimal size
		GUIElementBase::_updateOptimalLayoutSizes();

		if(mChildren.size() != mOptimalSizes.size())
			mOptimalSizes.resize(mChildren.size());

		mOptimalWidth = 0;
		mOptimalHeight = 0;

		UINT32 childIdx = 0;
		for(auto& child : mChildren)
		{
			UINT32 optimalWidth = 0;
			UINT32 optimalHeight = 0;

			if(child->_getType() == GUIElementBase::Type::FixedSpace)
			{
				GUIFixedSpace* fixedSpace = static_cast<GUIFixedSpace*>(child);
				optimalHeight = fixedSpace->getSize();
			}
			else if(child->_getType() == GUIElementBase::Type::Element)
			{
				GUIElement* element = static_cast<GUIElement*>(child);
				const GUILayoutOptions& layoutOptions = element->_getLayoutOptions();

				Vector2I optimalSize;
				if(!layoutOptions.fixedWidth || !layoutOptions.fixedHeight)
					optimalSize = child->_getOptimalSize();

				if(layoutOptions.fixedHeight)
				{
					optimalHeight = layoutOptions.height;
				}
				else
				{
					optimalHeight = optimalSize.y;

					if(layoutOptions.minHeight > 0)
						optimalHeight = std::max(layoutOptions.minHeight, optimalHeight);

					if(layoutOptions.maxHeight > 0)
						optimalHeight = std::min(layoutOptions.maxHeight, optimalHeight);
				}

				if(layoutOptions.fixedWidth)
					optimalWidth = layoutOptions.width;
				else
				{
					optimalWidth = optimalSize.x;

					if(layoutOptions.minWidth > 0)
						optimalWidth = std::max(layoutOptions.minWidth, optimalWidth);

					if(layoutOptions.maxWidth > 0)
						optimalWidth = std::min(layoutOptions.maxWidth, optimalWidth);
				}
			}
			else if(child->_getType() == GUIElementBase::Type::Layout)
			{
				GUILayout* layout = static_cast<GUILayout*>(child);
				optimalWidth = layout->_getOptimalSize().x;
				optimalHeight = layout->_getOptimalSize().y;
			}

			UINT32 paddingX = child->_getPadding().left + child->_getPadding().right;
			UINT32 paddingY = child->_getPadding().top + child->_getPadding().bottom;

			mOptimalSizes[childIdx].y = optimalHeight;
			mOptimalHeight += optimalHeight + paddingY;

			mOptimalSizes[childIdx].x = optimalWidth;
			mOptimalWidth = std::max(mOptimalWidth, optimalWidth + paddingX);

			childIdx++;
		}
	}

	void GUILayoutY::_updateLayoutInternal(INT32 x, INT32 y, UINT32 width, UINT32 height, RectI clipRect, UINT8 widgetDepth, UINT16 areaDepth)
	{
		UINT32 totalOptimalSize = _getOptimalSize().y;
		UINT32 totalNonClampedSize = 0;
		UINT32 numNonClampedElements = 0;
		UINT32 numFlexibleSpaces = 0;

		bool* processedElements = stackAllocN<bool>((UINT32)mChildren.size());
		memset(processedElements, 0, mChildren.size() * sizeof(bool));

		UINT32* elementSizes = stackAllocN<UINT32>((UINT32)mChildren.size());
		memset(elementSizes, 0, mChildren.size() * sizeof(UINT32));

		float* elementScaleWeights = stackAllocN<float>((UINT32)mChildren.size());
		memset(elementScaleWeights, 0, mChildren.size() * sizeof(float));

		// Set initial sizes, count number of children per type and mark fixed elements as already processed
		UINT32 childIdx = 0;
		for(auto& child : mChildren)
		{
			elementSizes[childIdx] = mOptimalSizes[childIdx].y;

			if(child->_getType() == GUIElementBase::Type::FixedSpace)
			{
				processedElements[childIdx] = true;
			}
			else if(child->_getType() == GUIElementBase::Type::Element)
			{
				GUIElement* element = static_cast<GUIElement*>(child);
				const GUILayoutOptions& layoutOptions = element->_getLayoutOptions();

				if(layoutOptions.fixedHeight)
					processedElements[childIdx] = true;
				else
				{
					numNonClampedElements++;
					totalNonClampedSize += elementSizes[childIdx];
				}
			}
			else if(child->_getType() == GUIElementBase::Type::Layout)
			{
				numNonClampedElements++;
				totalNonClampedSize += elementSizes[childIdx];
			}
			else if(child->_getType() == GUIElementBase::Type::FlexibleSpace)
			{
				numFlexibleSpaces++;
				numNonClampedElements++;
			}

			childIdx++;
		}

		// If there is some room left, calculate flexible space sizes (since they will fill up all that extra room)
		if(height > totalOptimalSize)
		{
			UINT32 extraSize = height - totalOptimalSize;
			UINT32 remainingSize = extraSize;

			// Flexible spaces always expand to fill up all unused space
			if(numFlexibleSpaces > 0)
			{
				float avgSize = remainingSize / (float)numFlexibleSpaces;

				childIdx = 0;
				for(auto& child : mChildren)
				{
					if(processedElements[childIdx])
					{
						childIdx++;
						continue;
					}

					UINT32 extraHeight = std::min((UINT32)Math::ceilToInt(avgSize), remainingSize);
					UINT32 elementHeight = elementSizes[childIdx] + extraHeight;

					// Clamp if needed
					if(child->_getType() == GUIElementBase::Type::FlexibleSpace)
					{
						processedElements[childIdx] = true;
						numNonClampedElements--;
						elementSizes[childIdx] = elementHeight;

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
		for(auto& child : mChildren)
		{
			if(processedElements[childIdx])
			{
				childIdx++;
				continue;
			}

			elementScaleWeights[childIdx] = invOptimalSize * elementSizes[childIdx];

			childIdx++;
		}

		// Our optimal size is larger than maximum allowed, so we need to reduce size of some elements
		if(totalOptimalSize > height)
		{
			UINT32 extraSize = totalOptimalSize - height;
			UINT32 remainingSize = extraSize;

			// Iterate until we reduce everything so it fits, while maintaining
			// equal average sizes using the weights we calculated earlier
			while(remainingSize > 0 && numNonClampedElements > 0)
			{
				UINT32 totalRemainingSize = remainingSize;

				childIdx = 0;
				for(auto& child : mChildren)
				{
					if(processedElements[childIdx])
					{
						childIdx++;
						continue;
					}

					float avgSize = totalRemainingSize * elementScaleWeights[childIdx];

					UINT32 extraHeight = std::min((UINT32)Math::ceilToInt(avgSize), remainingSize);
					UINT32 elementHeight = (UINT32)std::max(0, (INT32)elementSizes[childIdx] - (INT32)extraHeight);

					// Clamp if needed
					if(child->_getType() == GUIElementBase::Type::Element)
					{
						GUIElement* element = static_cast<GUIElement*>(child);
						const GUILayoutOptions& layoutOptions = element->_getLayoutOptions();

						if(elementHeight == 0)
						{
							processedElements[childIdx] = true;
							numNonClampedElements--;
						}
						else if(layoutOptions.minHeight > 0 && elementHeight < layoutOptions.minHeight)
						{
							elementHeight = layoutOptions.minHeight;

							processedElements[childIdx] = true;
							numNonClampedElements--;
						}

						extraHeight = elementSizes[childIdx] - elementHeight;
						elementSizes[childIdx] = elementHeight;
						remainingSize = (UINT32)std::max(0, (INT32)remainingSize - (INT32)extraHeight);
					}
					else if(child->_getType() == GUIElementBase::Type::Layout)
					{
						if(elementHeight == 0)
						{
							processedElements[childIdx] = true;
							numNonClampedElements--;
						}

						extraHeight = elementSizes[childIdx] - elementHeight;
						elementSizes[childIdx] = elementHeight;
						remainingSize = (UINT32)std::max(0, (INT32)remainingSize - (INT32)extraHeight);
					}
					else if(child->_getType() == GUIElementBase::Type::FlexibleSpace)
					{
						elementSizes[childIdx] = 0;
						processedElements[childIdx] = true;
						numNonClampedElements--;
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
			while(remainingSize > 0 && numNonClampedElements > 0)
			{
				UINT32 totalRemainingSize = remainingSize;

				childIdx = 0;
				for(auto& child : mChildren)
				{
					if(processedElements[childIdx])
					{
						childIdx++;
						continue;
					}

					float avgSize = totalRemainingSize * elementScaleWeights[childIdx];
					UINT32 extraHeight = std::min((UINT32)Math::ceilToInt(avgSize), remainingSize);
					UINT32 elementHeight = elementSizes[childIdx] + extraHeight;

					// Clamp if needed
					if(child->_getType() == GUIElementBase::Type::Element)
					{
						GUIElement* element = static_cast<GUIElement*>(child);
						const GUILayoutOptions& layoutOptions = element->_getLayoutOptions();

						if(elementHeight == 0)
						{
							processedElements[childIdx] = true;
							numNonClampedElements--;
						}
						else if(layoutOptions.maxHeight > 0 && elementHeight > layoutOptions.maxHeight)
						{
							elementHeight = layoutOptions.maxHeight;

							processedElements[childIdx] = true;
							numNonClampedElements--;
						}

						extraHeight = elementHeight - elementSizes[childIdx];
						elementSizes[childIdx] = elementHeight;
						remainingSize = (UINT32)std::max(0, (INT32)remainingSize - (INT32)extraHeight);
					}
					else if(child->_getType() == GUIElementBase::Type::Layout)
					{
						elementSizes[childIdx] = elementHeight;
						remainingSize = (UINT32)std::max(0, (INT32)remainingSize - (INT32)extraHeight);
					}
					else if(child->_getType() == GUIElementBase::Type::FlexibleSpace)
					{
						processedElements[childIdx] = true;
						numNonClampedElements--;
					}

					childIdx++;
				}
			}
		}

		// Now that we have all the sizes, actually assign them
		// Also assign offsets, clip rectangles and depth
		UINT32 yOffset = 0;
		childIdx = 0;

		mActualWidth = 0;
		mActualHeight = 0;
		for(auto& child : mChildren)
		{
			UINT32 elemHeight = elementSizes[childIdx];
			yOffset += child->_getPadding().top;
			
			if(child->_getType() == GUIElementBase::Type::Element)
			{
				GUIElement* element = static_cast<GUIElement*>(child);
				UINT32 elemWidth = mOptimalSizes[childIdx].x;
				const GUILayoutOptions& layoutOptions = element->_getLayoutOptions();
				if(!layoutOptions.fixedWidth)
				{
					elemWidth = width;
					if(layoutOptions.minWidth > 0 && elemWidth < layoutOptions.minWidth)
						elemWidth = layoutOptions.minWidth;

					if(layoutOptions.maxWidth > 0 && elemWidth > layoutOptions.maxWidth)
						elemWidth = layoutOptions.maxWidth;
				}

				element->_setWidth(elemWidth);
				element->_setHeight(elemHeight);

				UINT32 xPadding = element->_getPadding().left + element->_getPadding().right;
				INT32 xOffset = Math::ceilToInt((INT32)(width - (INT32)(element->_getWidth() + xPadding)) * 0.5f);
				xOffset = std::max(0, xOffset);

				Vector2I offset(x + xOffset, y + yOffset);
				element->_setOffset(offset);
				element->_setWidgetDepth(widgetDepth);
				element->_setAreaDepth(areaDepth);

				RectI elemClipRect(clipRect.x - offset.x, clipRect.y - offset.y, clipRect.width, clipRect.height);
				element->_setClipRect(elemClipRect);

				RectI newClipRect(offset.x, offset.y, elemWidth, elemHeight);
				newClipRect.clip(clipRect);
				element->_updateLayoutInternal(offset.x, offset.y, elemWidth, elemHeight, newClipRect, widgetDepth, areaDepth);

				mActualWidth = std::max(width, elemWidth);
			}
			else if(child->_getType() == GUIElementBase::Type::Layout)
			{
				GUILayout* layout = static_cast<GUILayout*>(child);

				RectI newClipRect(x, y + yOffset, width, elemHeight);
				newClipRect.clip(clipRect);
				layout->_updateLayoutInternal(x, y + yOffset, width, elemHeight, newClipRect, widgetDepth, areaDepth);

				mActualWidth = std::max(width, layout->_getActualWidth());

				// It's possible all elements didn't fit in the child layout size we provided, in which case adjust our measurements
				elemHeight = layout->_getActualHeight();
			}

			mActualHeight += elemHeight + child->_getPadding().top + child->_getPadding().bottom;
			yOffset += elemHeight + child->_getPadding().bottom;
			childIdx++;
		}

		stackDeallocLast(elementScaleWeights);
		stackDeallocLast(elementSizes);
		stackDeallocLast(processedElements);

		_markAsClean();
	}
}