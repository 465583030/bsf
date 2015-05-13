#include "BsTextureImportOptions.h"
#include "BsTextureImportOptionsRTTI.h"

namespace BansheeEngine
{
	TextureImportOptions::TextureImportOptions()
		:mFormat(PF_B8G8R8A8), mGenerateMips(false), mMaxMip(0), 
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