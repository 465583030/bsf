//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsTextureImportOptions.h"
#include "BsTextureImportOptionsRTTI.h"

namespace bs
{
	TextureImportOptions::TextureImportOptions()
		: mFormat(PF_R8G8B8A8), mGenerateMips(false), mMaxMip(0), mCPUCached(false), mSRGB(false)
	{ }

	SPtr<TextureImportOptions> TextureImportOptions::create()
	{
		return bs_shared_ptr_new<TextureImportOptions>();
	}

	/************************************************************************/
	/* 								SERIALIZATION                      		*/
	/************************************************************************/
	RTTITypeBase* TextureImportOptions::getRTTIStatic()
	{
		return TextureImportOptionsRTTI::instance();
	}

	RTTITypeBase* TextureImportOptions::getRTTI() const
	{
		return TextureImportOptions::getRTTIStatic();
	}
}