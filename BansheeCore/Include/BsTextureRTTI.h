#pragma once

#include "BsCorePrerequisites.h"
#include "BsRTTIType.h"
#include "BsTexture.h"
#include "BsManagedDataBlock.h"
#include "BsMath.h"
#include "BsCoreApplication.h"
#include "BsCoreThread.h"
#include "BsRenderAPI.h"
#include "BsTextureManager.h"
#include "BsPixelData.h"

namespace BansheeEngine
{
	class BS_CORE_EXPORT TextureRTTI : public RTTIType<Texture, Resource, TextureRTTI>
	{
	private:
		UINT32& getSize(Texture* obj) { return obj->mSize; }
		void setSize(Texture* obj, UINT32& val) { obj->mSize = val; }

		UINT32& getWidth(Texture* obj) { return obj->mProperties.mWidth; }
		void setWidth(Texture* obj, UINT32& val) { obj->mProperties.mWidth = val; }

		UINT32& getHeight(Texture* obj) { return obj->mProperties.mHeight; }
		void setHeight(Texture* obj, UINT32& val) { obj->mProperties.mHeight = val; }

		UINT32& getDepth(Texture* obj) { return obj->mProperties.mDepth; }
		void setDepth(Texture* obj, UINT32& val) { obj->mProperties.mDepth = val; }

		UINT32& getNumMipmaps(Texture* obj) { return obj->mProperties.mNumMipmaps; }
		void setNumMipmaps(Texture* obj, UINT32& val) { obj->mProperties.mNumMipmaps = val; }

		bool& getHwGamma(Texture* obj) { return obj->mProperties.mHwGamma; }
		void setHwGamma(Texture* obj, bool& val) { obj->mProperties.mHwGamma = val; }

		UINT32& getMultisampleCount(Texture* obj) { return obj->mProperties.mMultisampleCount; }
		void setMultisampleCount(Texture* obj, UINT32& val) { obj->mProperties.mMultisampleCount = val; }

		TextureType& getTextureType(Texture* obj) { return obj->mProperties.mTextureType; }
		void setTextureType(Texture* obj, TextureType& val) { obj->mProperties.mTextureType = val; }

		PixelFormat& getFormat(Texture* obj) { return obj->mProperties.mFormat; }
		void setFormat(Texture* obj, PixelFormat& val) { obj->mProperties.mFormat = val; }

		INT32& getUsage(Texture* obj) { return obj->mProperties.mUsage; }
		void setUsage(Texture* obj, INT32& val) 
		{ 
			// Render target and depth stencil texture formats are for in-memory use only
			// and don't make sense when serialized
			if (val == TU_DEPTHSTENCIL || val == TU_RENDERTARGET)
				obj->mProperties.mUsage = TU_STATIC;
			else
				obj->mProperties.mUsage = val;
		}

#define BS_ADD_PLAINFIELD(name, id, parentType) \
	addPlainField(#name, id##, &##parentType##::get##name, &##parentType##::Set##name);

		PixelDataPtr getPixelData(Texture* obj, UINT32 idx)
		{
			UINT32 face = (size_t)Math::floor(idx / (float)(obj->mProperties.getNumMipmaps() + 1));
			UINT32 mipmap = idx % (obj->mProperties.getNumMipmaps() + 1);

			UINT32 subresourceIdx = obj->mProperties.mapToSubresourceIdx(face, mipmap);
			PixelDataPtr pixelData = obj->mProperties.allocateSubresourceBuffer(subresourceIdx);

			obj->readSubresource(gCoreAccessor(), subresourceIdx, pixelData);
			gCoreAccessor().submitToCoreThread(true);

			return pixelData;
		}

		void setPixelData(Texture* obj, UINT32 idx, PixelDataPtr data)
		{
			Vector<PixelDataPtr>* pixelData = any_cast<Vector<PixelDataPtr>*>(obj->mRTTIData);

			(*pixelData)[idx] = data;
		}

		UINT32 getPixelDataArraySize(Texture* obj)
		{
			return obj->mProperties.getNumFaces() * (obj->mProperties.getNumMipmaps() + 1);
		}

		void setPixelDataArraySize(Texture* obj, UINT32 size)
		{
			Vector<PixelDataPtr>* pixelData = any_cast<Vector<PixelDataPtr>*>(obj->mRTTIData);

			pixelData->resize(size);
		}

	public:
		TextureRTTI()
		{
			addPlainField("mSize", 0, &TextureRTTI::getSize, &TextureRTTI::setSize);
			addPlainField("mHeight", 2, &TextureRTTI::getHeight, &TextureRTTI::setHeight);
			addPlainField("mWidth", 3, &TextureRTTI::getWidth, &TextureRTTI::setWidth);
			addPlainField("mDepth", 4, &TextureRTTI::getDepth, &TextureRTTI::setDepth);
			addPlainField("mNumMipmaps", 5, &TextureRTTI::getNumMipmaps, &TextureRTTI::setNumMipmaps);
			addPlainField("mHwGamma", 6, &TextureRTTI::getHwGamma, &TextureRTTI::setHwGamma);
			addPlainField("mMultisampleCount", 7, &TextureRTTI::getMultisampleCount, &TextureRTTI::setMultisampleCount);
			addPlainField("mTextureType", 9, &TextureRTTI::getTextureType, &TextureRTTI::setTextureType);
			addPlainField("mFormat", 10, &TextureRTTI::getFormat, &TextureRTTI::setFormat);
			addPlainField("mUsage", 11, &TextureRTTI::getUsage, &TextureRTTI::setUsage);

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

			const TextureProperties texProps = texture->getProperties();

			// A bit clumsy initializing with already set values, but I feel its better than complicating things and storing the values
			// in mRTTIData.
			texture->initialize();

			Vector<PixelDataPtr>* pixelData = any_cast<Vector<PixelDataPtr>*>(texture->mRTTIData);
			for(size_t i = 0; i < pixelData->size(); i++)
			{
				UINT32 face = (size_t)Math::floor(i / (float)(texProps.getNumMipmaps() + 1));
				UINT32 mipmap = i % (texProps.getNumMipmaps() + 1);

				UINT32 subresourceIdx = texProps.mapToSubresourceIdx(face, mipmap);

				texture->writeSubresource(gCoreAccessor(), subresourceIdx, pixelData->at(i), false);
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