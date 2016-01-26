#pragma once

#include "BsCorePrerequisites.h"
#include "BsRTTIType.h"
#include "BsTexture.h"
#include "BsManagedDataBlock.h"
#include "BsMath.h"
#include "BsCoreApplication.h"
#include "BsCoreThread.h"
#include "BsRenderSystem.h"
#include "BsTextureManager.h"
#include "BsPixelData.h"

namespace BansheeEngine
{
	class BS_CORE_EXPORT TextureRTTI : public RTTIType<Texture, Resource, TextureRTTI>
	{
	private:
		BS_SETGET_MEMBER(mSize, UINT32, Texture)
		BS_SETGET_MEMBER(mHeight, UINT32, Texture)
		BS_SETGET_MEMBER(mWidth, UINT32, Texture)
		BS_SETGET_MEMBER(mDepth, UINT32, Texture)
		BS_SETGET_MEMBER(mNumMipmaps, UINT32, Texture)
		BS_SETGET_MEMBER(mHwGamma, bool, Texture)
		BS_SETGET_MEMBER(mMultisampleCount, UINT32, Texture)
		BS_SETGET_MEMBER(mMultisampleHint, String, Texture)
		BS_SETGET_MEMBER(mTextureType, TextureType, Texture)
		BS_SETGET_MEMBER(mFormat, PixelFormat, Texture)
		BS_SETGET_MEMBER(mUsage, INT32, Texture)

		PixelDataPtr getPixelData(Texture* obj, UINT32 idx)
		{
			UINT32 face = (size_t)Math::floor(idx / (float)(obj->getNumMipmaps() + 1));
			UINT32 mipmap = idx % (obj->getNumMipmaps() + 1);

			UINT32 subresourceIdx = obj->mapToSubresourceIdx(face, mipmap);
			PixelDataPtr pixelData = obj->allocateSubresourceBuffer(subresourceIdx);

			GpuResourcePtr sharedTexPtr = std::static_pointer_cast<GpuResource>(obj->getThisPtr());

			// We want the data right away so queue directly to main core thread queue and block until we get it
			pixelData->_lock();
			gCoreThread().queueReturnCommand(
				std::bind(&RenderSystem::readSubresource, RenderSystem::instancePtr(), sharedTexPtr, subresourceIdx, 
				std::static_pointer_cast<GpuResourceData>(pixelData), std::placeholders::_1), true);

			return pixelData;
		}

		void setPixelData(Texture* obj, UINT32 idx, PixelDataPtr data)
		{
			Vector<PixelDataPtr>* pixelData = any_cast<Vector<PixelDataPtr>*>(obj->mRTTIData);

			(*pixelData)[idx] = data;
		}

		UINT32 getPixelDataArraySize(Texture* obj)
		{
			return obj->getNumFaces() * (obj->getNumMipmaps() + 1);
		}

		void setPixelDataArraySize(Texture* obj, UINT32 size)
		{
			Vector<PixelDataPtr>* pixelData = any_cast<Vector<PixelDataPtr>*>(obj->mRTTIData);

			pixelData->resize(size);
		}

	public:
		TextureRTTI()
		{
			BS_ADD_PLAINFIELD(mSize, 0, TextureRTTI)
			BS_ADD_PLAINFIELD(mHeight, 2, TextureRTTI)
			BS_ADD_PLAINFIELD(mWidth, 3, TextureRTTI)
			BS_ADD_PLAINFIELD(mDepth, 4, TextureRTTI)
			BS_ADD_PLAINFIELD(mNumMipmaps, 5, TextureRTTI)
			BS_ADD_PLAINFIELD(mHwGamma, 6, TextureRTTI)
			BS_ADD_PLAINFIELD(mMultisampleCount, 7, TextureRTTI)
			BS_ADD_PLAINFIELD(mMultisampleHint, 8, TextureRTTI)
			BS_ADD_PLAINFIELD(mTextureType, 9, TextureRTTI)
			BS_ADD_PLAINFIELD(mFormat, 10, TextureRTTI)
			BS_ADD_PLAINFIELD(mUsage, 11, TextureRTTI)

			addReflectablePtrArrayField("mPixelData", 12, &TextureRTTI::getPixelData, &TextureRTTI::getPixelDataArraySize, 
				&TextureRTTI::setPixelData, &TextureRTTI::setPixelDataArraySize);
		}

		virtual void onDeserializationStarted(IReflectable* obj)
		{
			Texture* texture = static_cast<Texture*>(obj);

			texture->mRTTIData = bs_new<Vector<PixelDataPtr>, PoolAlloc>();
		}

		virtual void onDeserializationEnded(IReflectable* obj)
		{
			Texture* texture = static_cast<Texture*>(obj);

			if(texture->mRTTIData.empty())
				return;

			// A bit clumsy initializing with already set values, but I feel its better than complicating things and storing the values
			// in mRTTIData.
			texture->initialize(texture->getTextureType(), texture->getWidth(), texture->getHeight(), texture->getDepth(), 
				texture->getNumMipmaps(), texture->getFormat(), texture->getUsage(), texture->isHardwareGammaEnabled(), 
				texture->getMultisampleCount(), texture->getMultisampleHint());

			Vector<PixelDataPtr>* pixelData = any_cast<Vector<PixelDataPtr>*>(texture->mRTTIData);
			for(size_t i = 0; i < pixelData->size(); i++)
			{
				UINT32 face = (size_t)Math::floor(i / (float)(texture->getNumMipmaps() + 1));
				UINT32 mipmap = i % (texture->getNumMipmaps() + 1);

				UINT32 subresourceIdx = texture->mapToSubresourceIdx(face, mipmap);

				GpuResourcePtr sharedTexPtr = std::static_pointer_cast<GpuResource>(texture->getThisPtr());

				pixelData->at(i)->_lock();
				gCoreThread().queueReturnCommand(std::bind(&RenderSystem::writeSubresource, RenderSystem::instancePtr(), 
					sharedTexPtr, subresourceIdx, pixelData->at(i), false, std::placeholders::_1));
			}

			bs_delete<PoolAlloc>(pixelData);
			texture->mRTTIData = nullptr;	
		}

		virtual const String& getRTTIName()
		{
			static String name = "Texture";
			return name;
		}

		virtual UINT32 getRTTIId()
		{
			return TID_Texture;
		}

		virtual std::shared_ptr<IReflectable> newRTTIObject()
		{
			return TextureManager::instance()._createEmpty();
		}
	};
}