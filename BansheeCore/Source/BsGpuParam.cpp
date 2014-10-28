#include "BsGpuParam.h"
#include "BsGpuParams.h"

namespace BansheeEngine
{
	GpuDataParamBase::GpuDataParamBase()
		:mParamDesc(nullptr)
	{ }

	GpuDataParamBase::GpuDataParamBase(GpuParamDataDesc* paramDesc, const std::shared_ptr<GpuParamsInternalData>& internalData)
		:mParamDesc(paramDesc), mInternalData(internalData)
	{ }

	bool GpuDataParamBase::isDestroyed() const
	{
		return mInternalData->mIsDestroyed;
	}

	GpuParamBlockPtr GpuDataParamBase::getParamBlock(UINT32 slotIdx) const
	{
		return mInternalData->mParamBlocks[slotIdx];
	}

	bool GpuDataParamBase::getTransposeMatrices() const
	{
		return mInternalData->mTransposeMatrices;
	}

	void GpuDataParamBase::markCoreDirty() 
	{ 
		mInternalData->mCoreDirtyFlags = 0xFFFFFFFF; 
	}

	/************************************************************************/
	/* 									STRUCT	                     		*/
	/************************************************************************/

	GpuParamStruct::GpuParamStruct()
		:mParamDesc(nullptr)
	{ }

	GpuParamStruct::GpuParamStruct(GpuParamDataDesc* paramDesc, const std::shared_ptr<GpuParamsInternalData>& internalData)
		:mParamDesc(paramDesc), mInternalData(internalData)
	{ }

	void GpuParamStruct::set(const void* value, UINT32 sizeBytes, UINT32 arrayIdx)
	{
		if (mInternalData == nullptr)
			return;

		if (mInternalData->mIsDestroyed)
			BS_EXCEPT(InternalErrorException, "Trying to access a destroyed gpu parameter.");

		UINT32 elementSizeBytes = mParamDesc->elementSize * sizeof(UINT32);

#if BS_DEBUG_MODE
		if(sizeBytes > elementSizeBytes)
		{
			LOGWRN("Provided element size larger than maximum element size. Maximum size: " + 
				toString(elementSizeBytes) + ". Supplied size: " + toString(sizeBytes));
		}

		if (arrayIdx >= mParamDesc->arraySize)
		{
			BS_EXCEPT(InvalidParametersException, "Array index out of range. Array size: " + 
				toString(mParamDesc->arraySize) + ". Requested size: " + toString(arrayIdx));
		}
#endif

		sizeBytes = std::min(elementSizeBytes, sizeBytes);

		GpuParamBlockPtr paramBlock = mInternalData->mParamBlocks[mParamDesc->paramBlockSlot];
		paramBlock->write((mParamDesc->cpuMemOffset + arrayIdx * mParamDesc->arrayElementStride) * sizeof(UINT32), value, sizeBytes);

		// Set unused bytes to 0
		if(sizeBytes < elementSizeBytes)
		{
			UINT32 diffSize = elementSizeBytes - sizeBytes;
			paramBlock->zeroOut((mParamDesc->cpuMemOffset + arrayIdx * mParamDesc->arrayElementStride)  * sizeof(UINT32)+sizeBytes, diffSize);
		}

		mInternalData->mCoreDirtyFlags = 0xFFFFFFFF;
	}

	void GpuParamStruct::get(void* value, UINT32 sizeBytes, UINT32 arrayIdx)
	{
		if (mInternalData == nullptr)
			return;

		if (mInternalData->mIsDestroyed)
			BS_EXCEPT(InternalErrorException, "Trying to access a destroyed gpu parameter.");

		UINT32 elementSizeBytes = mParamDesc->elementSize * sizeof(UINT32);

#if BS_DEBUG_MODE
		if(sizeBytes > elementSizeBytes)
		{
			LOGWRN("Provided element size larger than maximum element size. Maximum size: " + 
				toString(elementSizeBytes) + ". Supplied size: " + toString(sizeBytes));
		}

		if (arrayIdx >= mParamDesc->arraySize)
		{
			BS_EXCEPT(InvalidParametersException, "Array index out of range. Array size: " + 
				toString(mParamDesc->arraySize) + ". Requested size: " + toString(arrayIdx));
		}
#endif
		sizeBytes = std::min(elementSizeBytes, sizeBytes);

		GpuParamBlockPtr paramBlock = mInternalData->mParamBlocks[mParamDesc->paramBlockSlot];
		paramBlock->read((mParamDesc->cpuMemOffset + arrayIdx * mParamDesc->arrayElementStride) * sizeof(UINT32), value, sizeBytes);
	}

	UINT32 GpuParamStruct::getElementSize() const
	{
		if (mInternalData == nullptr)
			return 0;

		if(mInternalData->mIsDestroyed)
			BS_EXCEPT(InternalErrorException, "Trying to access a destroyed gpu parameter.");

		return mParamDesc->elementSize * sizeof(UINT32);
	}

	/************************************************************************/
	/* 								TEXTURE		                     		*/
	/************************************************************************/

	GpuParamTexture::GpuParamTexture()
		:mParamDesc(nullptr)
	{ }

	GpuParamTexture::GpuParamTexture(GpuParamObjectDesc* paramDesc, const std::shared_ptr<GpuParamsInternalData>& internalData)
		: mParamDesc(paramDesc), mInternalData(internalData)
	{ }

	void GpuParamTexture::set(const HTexture& texture)
	{
		if (mInternalData == nullptr)
			return;

		if (mInternalData->mIsDestroyed)
			BS_EXCEPT(InternalErrorException, "Trying to access a destroyed gpu parameter.");

		mInternalData->mTextures[mParamDesc->slot] = texture;
		mInternalData->mTextureInfo[mParamDesc->slot].isLoadStore = false;
		mInternalData->mCoreDirtyFlags = 0xFFFFFFFF;
	}

	HTexture GpuParamTexture::get()
	{
		if (mInternalData == nullptr)
			return HTexture();

		if (mInternalData->mIsDestroyed)
			BS_EXCEPT(InternalErrorException, "Trying to access a destroyed gpu parameter.");

		return mInternalData->mTextures[mParamDesc->slot];
	}

	/************************************************************************/
	/* 							LOAD/STORE TEXTURE		                    */
	/************************************************************************/

	GpuParamLoadStoreTexture::GpuParamLoadStoreTexture()
		:mParamDesc(nullptr)
	{ }

	GpuParamLoadStoreTexture::GpuParamLoadStoreTexture(GpuParamObjectDesc* paramDesc, const std::shared_ptr<GpuParamsInternalData>& internalData)
		: mParamDesc(paramDesc), mInternalData(internalData)
	{ }

	void GpuParamLoadStoreTexture::set(const HTexture& texture, const TextureSurface& surface)
	{
		if (mInternalData == nullptr)
			return;

		if (mInternalData->mIsDestroyed)
			BS_EXCEPT(InternalErrorException, "Trying to access a destroyed gpu parameter.");

		mInternalData->mTextures[mParamDesc->slot] = texture;
		mInternalData->mTextureInfo[mParamDesc->slot].isLoadStore = true;
		mInternalData->mTextureInfo[mParamDesc->slot].surface = surface;
		mInternalData->mCoreDirtyFlags = 0xFFFFFFFF;
	}

	HTexture GpuParamLoadStoreTexture::get()
	{
		if (mInternalData == nullptr)
			return HTexture();

		if (mInternalData->mIsDestroyed)
			BS_EXCEPT(InternalErrorException, "Trying to access a destroyed gpu parameter.");

		return mInternalData->mTextures[mParamDesc->slot];
	}

	/************************************************************************/
	/* 								SAMPLER STATE                      		*/
	/************************************************************************/

	GpuParamSampState::GpuParamSampState()
		:mParamDesc(nullptr)
	{ }

	GpuParamSampState::GpuParamSampState(GpuParamObjectDesc* paramDesc, const std::shared_ptr<GpuParamsInternalData>& internalData)
		: mParamDesc(paramDesc), mInternalData(internalData)
	{ }

	void GpuParamSampState::set(const HSamplerState& samplerState)
	{
		if (mInternalData == nullptr)
			return;

		if (mInternalData->mIsDestroyed)
			BS_EXCEPT(InternalErrorException, "Trying to access a destroyed gpu parameter.");

		mInternalData->mSamplerStates[mParamDesc->slot] = samplerState;
		mInternalData->mCoreDirtyFlags = 0xFFFFFFFF;
	}

	HSamplerState GpuParamSampState::get()
	{
		if (mInternalData == nullptr)
			return HSamplerState();

		if (mInternalData->mIsDestroyed)
			BS_EXCEPT(InternalErrorException, "Trying to access a destroyed gpu parameter.");

		return mInternalData->mSamplerStates[mParamDesc->slot];
	}
}