//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsTextureImportOptions.h"
#include "BsTextureImportOptionsRTTI.h"

namespace BansheeEngine
{
	TextureImportOptions::TextureImportOptions()
		:mFormat(PF_R8G8B8A8), mGenerateMips(true), mMaxMip(0), 
		mCPUReadable(false), mSRGB(false)
	{ }

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