#pragma once

#include "BsCorePrerequisites.h"

namespace BansheeEngine
{
	/**
	 * @brief	Kerning pair representing extra or less offset between
	 *			a specific pair of characters.
	 */
	struct KerningPair
	{
		UINT32 otherCharId;
		INT32 amount;
	};

	/**
	 * @brief	Describes a single character in a font.
	 */
	struct CHAR_DESC
	{
		UINT32 charId; /**< Character ID, corresponding to a Unicode key. */
		UINT32 page; /**< Index of the texture the character is located on. */
		float uvX, uvY; /**< Texture coordinates of the character in the page texture. */
		float uvWidth, uvHeight; /**< Width/height of the character in texture coordinates. */
		UINT32 width, height; /**< Width/height of the character in pixels. */
		INT32 xOffset, yOffset; /**< Determines offset for the visible portion of the character in pixels. */
		INT32 xAdvance, yAdvance; /**< Determines how much to advance the pen after writing this character. In pixels. */

		Vector<KerningPair> kerningPairs; /**< Pairs that determine if certain character pairs should be closer or father together. e.g. "AV" combination */
	};

	/**
	 * @brief	Describes a font.
	 */
	struct FONT_DESC
	{
		Map<UINT32, CHAR_DESC> characters; /**< All characters in the font referenced by character ID. */
		INT32 baselineOffset; /**< Y offset to the baseline on which the characters are placed. In pixels. */
		UINT32 lineHeight; /**< Height of a single line of the font. In pixels. */
		CHAR_DESC missingGlyph; /**< Character index to use when data for a character is missing. */
		UINT32 spaceWidth; /**< Determines width of the space in pixels. */
	};

	// Make CHAR_DESC serializable
	template<> struct RTTIPlainType<CHAR_DESC>
	{	
		enum { id = TID_CHAR_DESC }; enum { hasDynamicSize = 1 };

		static void toMemory(const CHAR_DESC& data, char* memory)
		{ 
			UINT32 size = getDynamicSize(data);

			memcpy(memory, &size, sizeof(UINT32));
			memory += sizeof(UINT32);

			memory = rttiWriteElem(data.charId, memory);
			memory = rttiWriteElem(data.page, memory);
			memory = rttiWriteElem(data.uvX, memory);
			memory = rttiWriteElem(data.uvY, memory);
			memory = rttiWriteElem(data.uvWidth, memory);
			memory = rttiWriteElem(data.uvHeight, memory);
			memory = rttiWriteElem(data.width, memory);
			memory = rttiWriteElem(data.height, memory);
			memory = rttiWriteElem(data.xOffset, memory);
			memory = rttiWriteElem(data.yOffset, memory);
			memory = rttiWriteElem(data.xAdvance, memory);
			memory = rttiWriteElem(data.yAdvance, memory);
			memory = rttiWriteElem(data.kerningPairs, memory);
		}

		static UINT32 fromMemory(CHAR_DESC& data, char* memory)
		{ 
			UINT32 size;
			memcpy(&size, memory, sizeof(UINT32)); 
			memory += sizeof(UINT32);

			memory = rttiReadElem(data.charId, memory);
			memory = rttiReadElem(data.page, memory);
			memory = rttiReadElem(data.uvX, memory);
			memory = rttiReadElem(data.uvY, memory);
			memory = rttiReadElem(data.uvWidth, memory);
			memory = rttiReadElem(data.uvHeight, memory);
			memory = rttiReadElem(data.width, memory);
			memory = rttiReadElem(data.height, memory);
			memory = rttiReadElem(data.xOffset, memory);
			memory = rttiReadElem(data.yOffset, memory);
			memory = rttiReadElem(data.xAdvance, memory);
			memory = rttiReadElem(data.yAdvance, memory);
			memory = rttiReadElem(data.kerningPairs, memory);

			return size;
		}

		static UINT32 getDynamicSize(const CHAR_DESC& data)	
		{ 
			UINT64 dataSize = sizeof(data.charId)
				+ sizeof(data.page)
				+ sizeof(data.uvX)
				+ sizeof(data.uvY)
				+ sizeof(data.uvWidth)
				+ sizeof(data.uvHeight)
				+ sizeof(data.width)
				+ sizeof(data.height)
				+ sizeof(data.xOffset)
				+ sizeof(data.yOffset)
				+ sizeof(data.xAdvance)
				+ sizeof(data.yAdvance)
				+ RTTIPlainType<Vector<KerningPair>>::getDynamicSize(data.kerningPairs);

			dataSize += sizeof(UINT32);

			return (UINT32)dataSize;
		}	
	}; 

	// Make FONT_DESC serializable
	template<> struct RTTIPlainType<FONT_DESC>
	{	
		enum { id = TID_FONT_DESC }; enum { hasDynamicSize = 1 };

		static void toMemory(const FONT_DESC& data, char* memory)
		{ 
			UINT32 size = getDynamicSize(data);

			memcpy(memory, &size, sizeof(UINT32));
			memory += sizeof(UINT32);
			
			RTTIPlainType<Map<UINT32, CHAR_DESC>>::toMemory(data.characters, memory);
			rttiWriteElem(data.baselineOffset, memory);
			rttiWriteElem(data.lineHeight, memory);
			rttiWriteElem(data.missingGlyph, memory);
			rttiWriteElem(data.spaceWidth, memory);
		}

		static UINT32 fromMemory(FONT_DESC& data, char* memory)
		{ 
			UINT32 size;
			memcpy(&size, memory, sizeof(UINT32)); 
			memory += sizeof(UINT32);

			RTTIPlainType<Map<UINT32, CHAR_DESC>>::fromMemory(data.characters, memory);
			rttiReadElem(data.baselineOffset, memory);
			rttiReadElem(data.lineHeight, memory);
			rttiReadElem(data.missingGlyph, memory);
			rttiReadElem(data.spaceWidth, memory);

			return size;
		}

		static UINT32 getDynamicSize(const FONT_DESC& data)	
		{ 
			UINT64 dataSize = sizeof(UINT32);
			dataSize += RTTIPlainType<Map<UINT32, CHAR_DESC>>::getDynamicSize(data.characters);
			dataSize += rttiGetElemSize(data.baselineOffset);
			dataSize += rttiGetElemSize(data.lineHeight);
			dataSize += rttiGetElemSize(data.missingGlyph);
			dataSize += rttiGetElemSize(data.spaceWidth);

			return (UINT32)dataSize;
		}	
	}; 
}