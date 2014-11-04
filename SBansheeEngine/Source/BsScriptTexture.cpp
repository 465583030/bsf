#include "BsScriptTexture.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"

namespace BansheeEngine
{
	ScriptTexture::ScriptTexture(MonoObject* instance)
		:ScriptObject(instance)
	{

	}

	void ScriptTexture::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_GetPixelFormat", &ScriptTexture::internal_getPixelFormat);
		metaData.scriptClass->addInternalCall("Internal_GetUsage", &ScriptTexture::internal_getUsage);
		metaData.scriptClass->addInternalCall("Internal_GetWidth", &ScriptTexture::internal_getWidth);
		metaData.scriptClass->addInternalCall("Internal_GetHeight", &ScriptTexture::internal_getHeight);
		metaData.scriptClass->addInternalCall("Internal_GetGammaCorrection", &ScriptTexture::internal_getGammaCorrection);
		metaData.scriptClass->addInternalCall("Internal_GetSampleCount", &ScriptTexture::internal_getSampleCount);
		metaData.scriptClass->addInternalCall("Internal_GetMipmapCount", &ScriptTexture::internal_getMipmapCount);
	}

	void ScriptTexture::internal_getPixelFormat(ScriptTexture* thisPtr, PixelFormat* value)
	{
		HTexture texture = static_resource_cast<Texture>(thisPtr->getNativeHandle());
		*value = texture->getFormat();
	}

	void ScriptTexture::internal_getUsage(ScriptTexture* thisPtr, TextureUsage* value)
	{
		HTexture texture = static_resource_cast<Texture>(thisPtr->getNativeHandle());
		*value = (TextureUsage)texture->getUsage();
	}

	void ScriptTexture::internal_getWidth(ScriptTexture* thisPtr, int* value)
	{
		HTexture texture = static_resource_cast<Texture>(thisPtr->getNativeHandle());
		*value = (TextureUsage)texture->getWidth();
	}

	void ScriptTexture::internal_getHeight(ScriptTexture* thisPtr, int* value)
	{
		HTexture texture = static_resource_cast<Texture>(thisPtr->getNativeHandle());
		*value = (TextureUsage)texture->getHeight();
	}

	void ScriptTexture::internal_getGammaCorrection(ScriptTexture* thisPtr, bool* value)
	{
		HTexture texture = static_resource_cast<Texture>(thisPtr->getNativeHandle());
		*value = (TextureUsage)texture->isHardwareGammaEnabled();
	}

	void ScriptTexture::internal_getSampleCount(ScriptTexture* thisPtr, int* value)
	{
		HTexture texture = static_resource_cast<Texture>(thisPtr->getNativeHandle());
		*value = (TextureUsage)texture->getMultisampleCount();
	}

	void ScriptTexture::internal_getMipmapCount(ScriptTexture* thisPtr, int* value)
	{
		HTexture texture = static_resource_cast<Texture>(thisPtr->getNativeHandle());
		*value = (TextureUsage)texture->getNumMipmaps();
	}
}