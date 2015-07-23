#include "BsImageSprite.h"
#include "BsSpriteTexture.h"
#include "BsGUIMaterialManager.h"
#include "BsSpriteTexture.h"
#include "BsTexture.h"

#include "BsProfilerCPU.h"

namespace BansheeEngine
{
	ImageSprite::ImageSprite()
	{

	}

	ImageSprite::~ImageSprite()
	{
		clearMesh();
	}

	void ImageSprite::update(const IMAGE_SPRITE_DESC& desc, UINT64 groupId)
	{
		const FontData* fontData = nullptr;
		if(desc.texture == nullptr || desc.texture->getTexture() == nullptr)
		{
			clearMesh();
			return;
		}

		// Actually generate a mesh
		if(mCachedRenderElements.size() < 1)
			mCachedRenderElements.resize(1);

		bool useScale9Grid = desc.borderLeft > 0 || desc.borderRight > 0 || 
			desc.borderTop > 0 || desc.borderBottom > 0;

		UINT32 numQuads = 1;
		if(useScale9Grid)
			numQuads = 9;

		UINT32 texPage = 0;
		SpriteRenderElement& renderElem = mCachedRenderElements[0];
		{
			UINT32 newNumQuads = numQuads;
			if(newNumQuads != renderElem.numQuads)
			{
				UINT32 oldVertexCount = renderElem.numQuads * 4;
				UINT32 oldIndexCount = renderElem.numQuads * 6;

				if(renderElem.vertices != nullptr) bs_deleteN<ScratchAlloc>(renderElem.vertices, oldVertexCount);
				if(renderElem.uvs != nullptr) bs_deleteN<ScratchAlloc>(renderElem.uvs, oldVertexCount);
				if(renderElem.indexes != nullptr) bs_deleteN<ScratchAlloc>(renderElem.indexes, oldIndexCount);

				renderElem.vertices = bs_newN<Vector2, ScratchAlloc>(newNumQuads * 4);
				renderElem.uvs = bs_newN<Vector2, ScratchAlloc>(newNumQuads * 4);
				renderElem.indexes = bs_newN<UINT32, ScratchAlloc>(newNumQuads * 6);
				renderElem.numQuads = newNumQuads;
			}

			const HTexture& tex = desc.texture->getTexture();

			bool getNewMaterial = false;
			if(renderElem.matInfo.material == nullptr)
				getNewMaterial = true;
			else
			{
				const GUIMaterialInfo* matInfo = nullptr;
				
				if (desc.transparent)
					matInfo = GUIMaterialManager::instance().findExistingImageMaterial(groupId, tex, desc.color);
				else
					matInfo = GUIMaterialManager::instance().findExistingNonAlphaImageMaterial(groupId, tex, desc.color);

				if(matInfo == nullptr)
				{
					getNewMaterial = true;
				}
				else
				{
					if(matInfo->material != renderElem.matInfo.material)
					{
						GUIMaterialManager::instance().releaseMaterial(renderElem.matInfo);
						getNewMaterial = true;
					}
				}
			}

			if (getNewMaterial)
			{
				if (desc.transparent)
					renderElem.matInfo = GUIMaterialManager::instance().requestImageMaterial(groupId, tex, desc.color);
				else
					renderElem.matInfo = GUIMaterialManager::instance().requestNonAlphaImageMaterial(groupId, tex, desc.color);
			}

			texPage++;
		}

		for(UINT32 i = 0; i < numQuads; i++)
		{
			renderElem.indexes[i * 6 + 0] = i * 4 + 0;
			renderElem.indexes[i * 6 + 1] = i * 4 + 1;
			renderElem.indexes[i * 6 + 2] = i * 4 + 2;
			renderElem.indexes[i * 6 + 3] = i * 4 + 1;
			renderElem.indexes[i * 6 + 4] = i * 4 + 3;
			renderElem.indexes[i * 6 + 5] = i * 4 + 2;
		}

		Vector2I offset = getAnchorOffset(desc.anchor, desc.width, desc.height);
		Vector2 uvOffset = desc.uvOffset;
		Vector2 uvScale = desc.uvScale;
		
		if(useScale9Grid)
		{
			UINT32 leftBorder = desc.borderLeft;
			UINT32 rightBorder = desc.borderRight;
			UINT32 topBorder = desc.borderTop;
			UINT32 bottomBorder = desc.borderBottom;

			float centerWidth = (float)std::max((INT32)0, (INT32)desc.width - (INT32)leftBorder - (INT32)rightBorder);
			float centerHeight = (float)std::max((INT32)0, (INT32)desc.height - (INT32)topBorder - (INT32)bottomBorder);

			float topCenterStart = (float)(offset.x + leftBorder);
			float topRightStart = (float)(topCenterStart + centerWidth);

			float middleStart = (float)(offset.y + topBorder);
			float bottomStart = (float)(middleStart + centerHeight);

			// Top left
			renderElem.vertices[0] = Vector2((float)offset.x, (float)offset.y);
			renderElem.vertices[1] = Vector2((float)offset.x + leftBorder, (float)offset.y);
			renderElem.vertices[2] = Vector2((float)offset.x, middleStart);
			renderElem.vertices[3] = Vector2((float)offset.x + leftBorder, middleStart);

			// Top center
			renderElem.vertices[4] = Vector2(topCenterStart, (float)offset.y);
			renderElem.vertices[5] = Vector2(topCenterStart + centerWidth, (float)offset.y);
			renderElem.vertices[6] = Vector2(topCenterStart, middleStart);
			renderElem.vertices[7] = Vector2(topCenterStart + centerWidth, middleStart);

			// Top right
			renderElem.vertices[8] = Vector2(topRightStart, (float)offset.y);
			renderElem.vertices[9] = Vector2(topRightStart + rightBorder, (float)offset.y);
			renderElem.vertices[10] = Vector2(topRightStart, middleStart);
			renderElem.vertices[11] = Vector2(topRightStart + rightBorder, middleStart);

			// Middle left
			renderElem.vertices[12] = Vector2((float)offset.x, middleStart);
			renderElem.vertices[13] = Vector2((float)offset.x + leftBorder, middleStart);
			renderElem.vertices[14] = Vector2((float)offset.x, bottomStart);
			renderElem.vertices[15] = Vector2((float)offset.x + leftBorder, bottomStart);

			// Middle center
			renderElem.vertices[16] = Vector2(topCenterStart, middleStart);
			renderElem.vertices[17] = Vector2(topCenterStart + centerWidth, middleStart);
			renderElem.vertices[18] = Vector2(topCenterStart, bottomStart);
			renderElem.vertices[19] = Vector2(topCenterStart + centerWidth, bottomStart);

			// Middle right
			renderElem.vertices[20] = Vector2(topRightStart, middleStart);
			renderElem.vertices[21] = Vector2(topRightStart + rightBorder, middleStart);
			renderElem.vertices[22] = Vector2(topRightStart, bottomStart);
			renderElem.vertices[23] = Vector2(topRightStart + rightBorder, bottomStart);

			// Bottom left
			renderElem.vertices[24] = Vector2((float)offset.x, bottomStart);
			renderElem.vertices[25] = Vector2((float)offset.x + leftBorder, bottomStart);
			renderElem.vertices[26] = Vector2((float)offset.x, bottomStart + bottomBorder);
			renderElem.vertices[27] = Vector2((float)offset.x + leftBorder, bottomStart + bottomBorder);

			// Bottom center
			renderElem.vertices[28] = Vector2(topCenterStart, bottomStart);
			renderElem.vertices[29] = Vector2(topCenterStart + centerWidth, bottomStart);
			renderElem.vertices[30] = Vector2(topCenterStart, bottomStart + bottomBorder);
			renderElem.vertices[31] = Vector2(topCenterStart + centerWidth, bottomStart + bottomBorder);

			// Bottom right
			renderElem.vertices[32] = Vector2(topRightStart, bottomStart);
			renderElem.vertices[33] = Vector2(topRightStart + rightBorder, bottomStart);
			renderElem.vertices[34] = Vector2(topRightStart, bottomStart + bottomBorder);
			renderElem.vertices[35] = Vector2(topRightStart + rightBorder, bottomStart + bottomBorder);

			float invWidth = 1.0f / (float)desc.texture->getTexture()->getProperties().getWidth();
			float invHeight = 1.0f / (float)desc.texture->getTexture()->getProperties().getHeight();

			float uvLeftBorder = desc.borderLeft * invWidth;
			float uvRightBorder = desc.borderRight * invWidth;
			float uvTopBorder = desc.borderTop * invHeight;
			float uvBottomBorder = desc.borderBottom * invHeight;

			float uvCenterWidth = std::max(0.0f, uvScale.x - uvLeftBorder - uvRightBorder);
			float uvCenterHeight = std::max(0.0f, uvScale.y - uvTopBorder - uvBottomBorder);

			float uvTopCenterStart = uvOffset.x + uvLeftBorder;
			float uvTopRightStart = uvTopCenterStart + uvCenterWidth;

			float uvMiddleStart = uvOffset.y + uvTopBorder;
			float uvBottomStart = uvMiddleStart + uvCenterHeight;

			// UV - Top left
			renderElem.uvs[0] = desc.texture->transformUV(Vector2(uvOffset.x, uvOffset.y));
			renderElem.uvs[1] = desc.texture->transformUV(Vector2(uvOffset.x + uvLeftBorder, uvOffset.y));
			renderElem.uvs[2] = desc.texture->transformUV(Vector2(uvOffset.x, uvOffset.y + uvTopBorder));
			renderElem.uvs[3] = desc.texture->transformUV(Vector2(uvOffset.x + uvLeftBorder, uvOffset.y + uvTopBorder));

			// UV - Top center
			renderElem.uvs[4] = desc.texture->transformUV(Vector2(uvTopCenterStart, uvOffset.y));
			renderElem.uvs[5] = desc.texture->transformUV(Vector2(uvTopCenterStart + uvCenterWidth, uvOffset.y));
			renderElem.uvs[6] = desc.texture->transformUV(Vector2(uvTopCenterStart, uvOffset.y + uvTopBorder));
			renderElem.uvs[7] = desc.texture->transformUV(Vector2(uvTopCenterStart + uvCenterWidth, uvOffset.y + uvTopBorder));

			// UV - Top right
			renderElem.uvs[8] = desc.texture->transformUV(Vector2(uvTopRightStart, uvOffset.y));
			renderElem.uvs[9] = desc.texture->transformUV(Vector2(uvTopRightStart + uvRightBorder, uvOffset.y));
			renderElem.uvs[10] = desc.texture->transformUV(Vector2(uvTopRightStart, uvOffset.y + uvTopBorder));
			renderElem.uvs[11] = desc.texture->transformUV(Vector2(uvTopRightStart + uvRightBorder, uvOffset.y + uvTopBorder));

			// UV - Middle left
			renderElem.uvs[12] = desc.texture->transformUV(Vector2(uvOffset.x, uvMiddleStart));
			renderElem.uvs[13] = desc.texture->transformUV(Vector2(uvOffset.x + uvLeftBorder, uvMiddleStart));
			renderElem.uvs[14] = desc.texture->transformUV(Vector2(uvOffset.x, uvMiddleStart + uvCenterHeight));
			renderElem.uvs[15] = desc.texture->transformUV(Vector2(uvOffset.x + uvLeftBorder, uvMiddleStart + uvCenterHeight));

			// UV - Middle center
			renderElem.uvs[16] = desc.texture->transformUV(Vector2(uvTopCenterStart, uvMiddleStart));
			renderElem.uvs[17] = desc.texture->transformUV(Vector2(uvTopCenterStart + uvCenterWidth, uvMiddleStart));
			renderElem.uvs[18] = desc.texture->transformUV(Vector2(uvTopCenterStart, uvMiddleStart + uvCenterHeight));
			renderElem.uvs[19] = desc.texture->transformUV(Vector2(uvTopCenterStart + uvCenterWidth, uvMiddleStart + uvCenterHeight));

			// UV - Middle right
			renderElem.uvs[20] = desc.texture->transformUV(Vector2(uvTopRightStart, uvMiddleStart));
			renderElem.uvs[21] = desc.texture->transformUV(Vector2(uvTopRightStart + uvRightBorder, uvMiddleStart));
			renderElem.uvs[22] = desc.texture->transformUV(Vector2(uvTopRightStart, uvMiddleStart + uvCenterHeight));
			renderElem.uvs[23] = desc.texture->transformUV(Vector2(uvTopRightStart + uvRightBorder, uvMiddleStart + uvCenterHeight));

			// UV - Bottom left
			renderElem.uvs[24] = desc.texture->transformUV(Vector2(uvOffset.x, uvBottomStart));
			renderElem.uvs[25] = desc.texture->transformUV(Vector2(uvOffset.x + uvLeftBorder, uvBottomStart));
			renderElem.uvs[26] = desc.texture->transformUV(Vector2(uvOffset.x, uvBottomStart + uvBottomBorder));
			renderElem.uvs[27] = desc.texture->transformUV(Vector2(uvOffset.x + uvLeftBorder, uvBottomStart + uvBottomBorder));

			// UV - Bottom center
			renderElem.uvs[28] = desc.texture->transformUV(Vector2(uvTopCenterStart, uvBottomStart));
			renderElem.uvs[29] = desc.texture->transformUV(Vector2(uvTopCenterStart + uvCenterWidth, uvBottomStart));
			renderElem.uvs[30] = desc.texture->transformUV(Vector2(uvTopCenterStart, uvBottomStart + uvBottomBorder));
			renderElem.uvs[31] = desc.texture->transformUV(Vector2(uvTopCenterStart + uvCenterWidth, uvBottomStart + uvBottomBorder));

			// UV - Bottom right
			renderElem.uvs[32] = desc.texture->transformUV(Vector2(uvTopRightStart, uvBottomStart));
			renderElem.uvs[33] = desc.texture->transformUV(Vector2(uvTopRightStart + uvRightBorder, uvBottomStart));
			renderElem.uvs[34] = desc.texture->transformUV(Vector2(uvTopRightStart, uvBottomStart + uvBottomBorder));
			renderElem.uvs[35] = desc.texture->transformUV(Vector2(uvTopRightStart + uvRightBorder, uvBottomStart + uvBottomBorder));
		}
		else
		{
			renderElem.vertices[0] = Vector2((float)offset.x, (float)offset.y);
			renderElem.vertices[1] = Vector2((float)offset.x + desc.width, (float)offset.y);
			renderElem.vertices[2] = Vector2((float)offset.x, (float)offset.y + desc.height);
			renderElem.vertices[3] = Vector2((float)offset.x + desc.width, (float)offset.y + desc.height);

			renderElem.uvs[0] = desc.texture->transformUV(Vector2(uvOffset.x, uvOffset.y));
			renderElem.uvs[1] = desc.texture->transformUV(Vector2(uvOffset.x + uvScale.x, uvOffset.y));
			renderElem.uvs[2] = desc.texture->transformUV(Vector2(uvOffset.x, uvOffset.y + uvScale.y));
			renderElem.uvs[3] = desc.texture->transformUV(Vector2(uvOffset.x + uvScale.x, uvOffset.y + uvScale.y));
		}

		updateBounds();
	}

	void ImageSprite::clearMesh()
	{
		for (auto& renderElem : mCachedRenderElements)
		{
			UINT32 vertexCount = renderElem.numQuads * 4;
			UINT32 indexCount = renderElem.numQuads * 6;

			if (renderElem.vertices != nullptr)
			{
				bs_deleteN<ScratchAlloc>(renderElem.vertices, vertexCount);
				renderElem.vertices = nullptr;
			}

			if (renderElem.uvs != nullptr)
			{
				bs_deleteN<ScratchAlloc>(renderElem.uvs, vertexCount);
				renderElem.uvs = nullptr;
			}

			if (renderElem.indexes != nullptr)
			{
				bs_deleteN<ScratchAlloc>(renderElem.indexes, indexCount);
				renderElem.indexes = nullptr;
			}

			if (renderElem.matInfo.material != nullptr)
			{
				GUIMaterialManager::instance().releaseMaterial(renderElem.matInfo);
			}
		}

		mCachedRenderElements.clear();
		updateBounds();
	}
}