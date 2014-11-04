#include "BsScriptTexture3D.h"
#include "BsScriptResourceManager.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsTexture.h"
#include "BsPixelUtil.h"
#include "BsException.h"
#include "BsScriptPixelData.h"
#include "BsScriptAsyncOp.h"
#include "BsCoreThread.h"

using namespace std::placeholders;

namespace BansheeEngine
{
	ScriptTexture3D::ScriptTexture3D(MonoObject* instance, const HTexture& texture)
		:ScriptObject(instance), mTexture(texture)
	{

	}

	void ScriptTexture3D::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptTexture3D::internal_createInstance);
		metaData.scriptClass->addInternalCall("Internal_GetPixels", &ScriptTexture3D::internal_getPixels);
		metaData.scriptClass->addInternalCall("Internal_GetGPUPixels", &ScriptTexture3D::internal_getGPUPixels);
		metaData.scriptClass->addInternalCall("Internal_SetPixels", &ScriptTexture3D::internal_setPixels);
	}

	void ScriptTexture3D::internal_createInstance(MonoObject* instance, PixelFormat format, UINT32 width,
		UINT32 height, UINT32 depth, TextureUsage usage, bool hasMipmaps, bool gammaCorrection)
	{
		int numMips = 0;
		if (hasMipmaps)
			numMips = PixelUtil::getMaxMipmaps(width, height, depth, format);

		HTexture texture = Texture::create(TEX_TYPE_3D, width, height, depth, numMips, format, usage, gammaCorrection);

		ScriptResourceManager::instance().createScriptTexture3D(instance, texture);
	}

	MonoObject* ScriptTexture3D::internal_getPixels(ScriptTexture3D* thisPtr, UINT32 mipLevel)
	{
		HTexture texture = thisPtr->mTexture;
		UINT32 subresourceIdx = texture->mapToSubresourceIdx(0, mipLevel);

		PixelDataPtr pixelData = thisPtr->mTexture->allocateSubresourceBuffer(subresourceIdx);

		thisPtr->mTexture->readDataSim(*pixelData, mipLevel);

		return ScriptPixelData::create(pixelData);
	}

	MonoObject* ScriptTexture3D::internal_getGPUPixels(ScriptTexture3D* thisPtr, UINT32 mipLevel)
	{
		HTexture texture = thisPtr->mTexture;
		UINT32 subresourceIdx = texture->mapToSubresourceIdx(0, mipLevel);

		PixelDataPtr readData = texture->allocateSubresourceBuffer(subresourceIdx);

		AsyncOp asyncOp = gCoreAccessor().readSubresource(texture.getInternalPtr(), subresourceIdx, readData);

		std::function<MonoObject*(const AsyncOp&, const PixelDataPtr&)> asyncOpToMono =
			[&](const AsyncOp& op, const PixelDataPtr& returnValue)
		{
			return ScriptPixelData::create(returnValue);
		};

		return ScriptAsyncOp::create(asyncOp, std::bind(asyncOpToMono, _1, readData));
	}

	void ScriptTexture3D::internal_setPixels(ScriptTexture3D* thisPtr, MonoObject* data, UINT32 mipLevel)
	{
		ScriptPixelData* scriptPixelData = ScriptPixelData::toNative(data);

		if (scriptPixelData != nullptr)
		{
			HTexture texture = thisPtr->mTexture;
			UINT32 subresourceIdx = texture->mapToSubresourceIdx(0, mipLevel);

			gCoreAccessor().writeSubresource(texture.getInternalPtr(), subresourceIdx, scriptPixelData->getInternalValue());
		}
	}

	void ScriptTexture3D::_onManagedInstanceDeleted()
	{
		mManagedInstance = nullptr;
		ScriptResourceManager::instance().destroyScriptResource(this);
	}

	void ScriptTexture3D::setNativeHandle(const HResource& resource)
	{
		mTexture = static_resource_cast<Texture>(resource);
	}
}