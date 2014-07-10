//__________________________ Banshee Project - A modern game development toolkit _________________________________//
//_____________________________________ www.banshee-project.com __________________________________________________//
//________________________ Copyright (c) 2014 Marko Pintera. All rights reserved. ________________________________//
#include "BsSpriteTexture.h"
#include "BsSpriteTextureRTTI.h"
#include "BsTexture.h"
#include "BsResources.h"

namespace BansheeEngine
{
	SpriteTexture::SpriteTexture(const Vector2& uvOffset, const Vector2& uvScale, const HTexture& texture)
		:Resource(false), mUVOffset(uvOffset), mUVScale(uvScale), mAtlasTexture(texture)
	{

	}

	const HTexture& SpriteTexture::getTexture() const 
	{ 
		return mAtlasTexture; 
	}

	Vector2 SpriteTexture::transformUV(const Vector2& uv) const
	{
		return mUVOffset + uv * mUVScale;
	}

	const HSpriteTexture& SpriteTexture::dummy()
	{
		static HSpriteTexture dummyTex = create(Texture::dummy());

		return dummyTex;
	}

	bool SpriteTexture::checkIsLoaded(const HSpriteTexture& tex)
	{
		return tex != nullptr && tex.isLoaded() && tex->getTexture() != nullptr && tex.isLoaded();
	}

	HSpriteTexture SpriteTexture::create(const HTexture& texture)
	{
		SpriteTexturePtr texturePtr = bs_core_ptr<SpriteTexture, PoolAlloc>
			(new (bs_alloc<SpriteTexture, PoolAlloc>()) SpriteTexture(Vector2(0.0f, 0.0f), Vector2(1.0f, 1.0f), texture));

		texturePtr->_setThisPtr(texturePtr);
		texturePtr->initialize();

		return static_resource_cast<SpriteTexture>(gResources()._createResourceHandle(texturePtr));
	}

	HSpriteTexture SpriteTexture::create(const Vector2& uvOffset, const Vector2& uvScale, const HTexture& texture)
	{
		SpriteTexturePtr texturePtr = bs_core_ptr<SpriteTexture, PoolAlloc>
			(new (bs_alloc<SpriteTexture, PoolAlloc>()) SpriteTexture(uvOffset, uvScale, texture));

		texturePtr->_setThisPtr(texturePtr);
		texturePtr->initialize();

		return static_resource_cast<SpriteTexture>(gResources()._createResourceHandle(texturePtr));
	}

	SpriteTexturePtr SpriteTexture::createEmpty()
	{
		SpriteTexturePtr texturePtr = bs_core_ptr<SpriteTexture, PoolAlloc>
			(new (bs_alloc<SpriteTexture, PoolAlloc>()) SpriteTexture(Vector2(0.0f, 0.0f), Vector2(1.0f, 1.0f), Texture::dummy()));

		texturePtr->_setThisPtr(texturePtr);
		texturePtr->initialize();

		return texturePtr;
	}

	RTTITypeBase* SpriteTexture::getRTTIStatic()
	{
		return SpriteTextureRTTI::instance();
	}

	RTTITypeBase* SpriteTexture::getRTTI() const
	{
		return SpriteTexture::getRTTIStatic();
	}
}