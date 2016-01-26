#include "BsScriptTexture2D.h"
#include "BsScriptResourceManager.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsTexture.h"
#include "BsPixelUtil.h"
#include "BsException.h"

namespace BansheeEngine
{
	ScriptTexture2D::ScriptTexture2D(MonoObject* instance, const HTexture& texture)
		:ScriptObject(instance), mTexture(texture)
	{

	}

	void ScriptTexture2D::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptTexture2D::internal_createInstance);
	}

	void ScriptTexture2D::internal_createInstance(MonoObject* instance, UINT32 format, UINT32 width, UINT32 height, bool hasMipmaps, bool gammaCorrection)
	{
		PixelFormat texFormat = PF_R8G8B8;
		switch(format)
		{
		case 0: // RGB
			texFormat = PF_R8G8B8;
			break;
		case 1: // RGBA
			texFormat = PF_R8G8B8A8;
			break;
		default:
			BS_EXCEPT(InvalidParametersException, "Unsupported texture format");
		}

		int numMips = 0;
		if(hasMipmaps)
			numMips = PixelUtil::getMaxMipmaps(width, height, 1, texFormat);

		HTexture texture = Texture::create(TEX_TYPE_2D, width, height, numMips, texFormat, TU_STATIC, gammaCorrection);

		ScriptResourceManager::instance().createScriptTexture(instance, texture);
	}

	void ScriptTexture2D::_onManagedInstanceDeleted()
	{
		mManagedInstance = nullptr;
		ScriptResourceManager::instance().destroyScriptResource(this);
	}

	void ScriptTexture2D::setNativeHandle(const HResource& resource) 
	{ 
		mTexture = static_resource_cast<Resource>(mTexture); 
	}
}