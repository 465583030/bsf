#include "BsGUIHelper.h"
#include "BsSpriteTexture.h"
#include "BsGUIElementStyle.h"
#include "BsGUIDimensions.h"
#include "BsTexture.h"

namespace BansheeEngine
{
	Vector2I GUIHelper::calcOptimalContentsSize(const Vector2I& contentSize, const GUIElementStyle& style, const GUIDimensions& dimensions)
	{
		UINT32 contentWidth = style.margins.left + style.margins.right + style.contentOffset.left + style.contentOffset.right;
		UINT32 contentHeight = style.margins.top + style.margins.bottom + style.contentOffset.top + style.contentOffset.bottom;

		return Vector2I(std::max((UINT32)contentSize.x, contentWidth), std::max((UINT32)contentSize.y, contentHeight));
	}

	Vector2I GUIHelper::calcOptimalContentsSize(const GUIContent& content, const GUIElementStyle& style, const GUIDimensions& dimensions)
	{
		Vector2I contentBounds = calcOptimalContentsSize((const WString&)content.getText(), style, dimensions);

		if(content.getImage() != nullptr)
		{
			contentBounds.x += content.getImage()->getWidth() + GUIContent::IMAGE_TEXT_SPACING;
			contentBounds.y = std::max(content.getImage()->getHeight(), (UINT32)contentBounds.y);
		}

		return contentBounds;
	}

	Vector2I GUIHelper::calcOptimalContentsSize(const WString& text, const GUIElementStyle& style, const GUIDimensions& dimensions)
	{
		UINT32 wordWrapWidth = 0;

		if(style.wordWrap)
			wordWrapWidth = dimensions.maxWidth;

		UINT32 contentWidth = style.margins.left + style.margins.right + style.contentOffset.left + style.contentOffset.right;
		UINT32 contentHeight = style.margins.top + style.margins.bottom + style.contentOffset.top + style.contentOffset.bottom;

		if(style.font != nullptr)
		{
			bs_frame_mark();

			TextData<FrameAlloc> textData(text, style.font, style.fontSize, wordWrapWidth, 0, style.wordWrap);

			contentWidth += textData.getWidth();
			contentHeight += textData.getNumLines() * textData.getLineHeight(); 

			bs_frame_clear();
		}

		return Vector2I(contentWidth, contentHeight);
	}
}