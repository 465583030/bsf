#include "BsMaterialParam.h"
#include "BsVector2I.h"
#include "BsVectorNI.h"
#include "BsMaterialParams.h"

namespace BansheeEngine
{
	template<class T>
	TMaterialDataParam<T, false>::TMaterialDataParam(const String& name, const SPtr<MaterialParams>& params,
		const SPtr<Vector<TGpuDataParam<T, false>>>& gpuParams)
		:mParamIndex(0), mArraySize(0), mMaterialParams(nullptr), mGPUParams(gpuParams)
	{
		if(params != nullptr)
		{
			const MaterialParams::ParamData* data = nullptr;
			auto result = params->getParamData(name, MaterialParams::ParamType::Data, 
				(GpuParamDataType)TGpuDataParamInfo<T>::TypeId, 0, &data);

			if (result == MaterialParams::GetParamResult::Success)
			{
				mMaterialParams = params;
				mParamIndex = data->index;
				mArraySize = data->arraySize;
			}
			else
				params->reportGetParamError(result, name, 0);
		}
	}

	template<class T>
	void TMaterialDataParam<T, false>::set(const T& value, UINT32 arrayIdx)
	{
		if (mMaterialParams == nullptr)
			return;

		if(arrayIdx >= mArraySize)
		{
			LOGWRN("Array index out of range. Provided index was " + toString(arrayIdx) + 
				" but array length is " + toString(mArraySize));
			return;
		}

		mMaterialParams->setDataParam(mParamIndex, arrayIdx, value);

		if (mGPUParams != nullptr)
		{
			for (auto& param : *mGPUParams)
				param.set(value, arrayIdx);
		}
	}

	template<class T>
	T TMaterialDataParam<T, false>::get(UINT32 arrayIdx)
	{
		T output = T();
		if (mMaterialParams == nullptr || arrayIdx >= mArraySize)
			return output;

		mMaterialParams->getDataParam(mParamIndex, arrayIdx, output);
		return output;
	}

	template<class T>
	TMaterialDataParam<T, true>::TMaterialDataParam(const SPtr<Vector<TGpuDataParam<T, true>>>& params)
		:mParams(params)
	{ }

	template<class T>
	void TMaterialDataParam<T, true>::set(const T& value, UINT32 arrayIdx)
	{
		if (mParams == nullptr)
			return;

		for (auto& param : *mParams)
			param.set(value, arrayIdx);
	}

	template<class T>
	T TMaterialDataParam<T, true>::get(UINT32 arrayIdx)
	{
		if (mParams == nullptr || mParams->size() == 0)
			return T();

		return (*mParams)[0].get(arrayIdx); // They should all have the same value
	}

	TMaterialParamStruct<false>::TMaterialParamStruct(const String& name, const SPtr<MaterialParams>& params, 
		const SPtr<Vector<TGpuParamStruct<false>>>& gpuParams)
		:mParamIndex(0), mArraySize(0), mMaterialParams(nullptr), mGPUParams(gpuParams)
	{
		if (params != nullptr)
		{
			const MaterialParams::ParamData* data = nullptr;
			auto result = params->getParamData(name, MaterialParams::ParamType::Data, GPDT_STRUCT, 0, &data);

			if (result == MaterialParams::GetParamResult::Success)
			{
				mMaterialParams = params;
				mParamIndex = data->index;
				mArraySize = data->arraySize;
			}
			else
				params->reportGetParamError(result, name, 0);
		}
	}

	void TMaterialParamStruct<false>::set(const void* value, UINT32 sizeBytes, UINT32 arrayIdx)
	{
		if (mMaterialParams == nullptr)
			return;

		if (arrayIdx >= mArraySize)
		{
			LOGWRN("Array index out of range. Provided index was " + toString(arrayIdx) +
				" but array length is " + toString(mArraySize));
			return;
		}

		mMaterialParams->setStructData(mParamIndex + arrayIdx, value, sizeBytes);

		if (mGPUParams != nullptr)
		{
			for (auto& param : *mGPUParams)
				param.set(value, sizeBytes, arrayIdx);
		}
	}

	void TMaterialParamStruct<false>::get(void* value, UINT32 sizeBytes, UINT32 arrayIdx)
	{
		if (mMaterialParams == nullptr || arrayIdx >= mArraySize)
			return;

		mMaterialParams->getStructData(mParamIndex + arrayIdx, value, sizeBytes);
	}

	UINT32 TMaterialParamStruct<false>::getElementSize() const
	{
		return mMaterialParams->getStructSize(mParamIndex);
	}

	TMaterialParamStruct<true>::TMaterialParamStruct(const SPtr<Vector<TGpuParamStruct<true>>>& params)
		:mParams(params)
	{ }

	void TMaterialParamStruct<true>::set(const void* value, UINT32 sizeBytes, UINT32 arrayIdx)
	{
		if (mParams == nullptr)
			return;

		for (auto& param : *mParams)
			param.set(value, sizeBytes, arrayIdx);
	}

	void TMaterialParamStruct<true>::get(void* value, UINT32 sizeBytes, UINT32 arrayIdx)
	{
		if (mParams == nullptr || mParams->size() == 0)
		{
			value = nullptr;
			return;
		}

		return (*mParams)[0].get(value, sizeBytes, arrayIdx); // They should all have the same value
	}

	UINT32 TMaterialParamStruct<true>::getElementSize() const
	{
		if (mParams == nullptr || mParams->size() == 0)
			return 0;

		return (*mParams)[0].getElementSize();
	}

	TMaterialParamTexture<false>::TMaterialParamTexture(const String& name, const SPtr<MaterialParams>& params, 
		const SPtr<Vector<TGpuParamTexture<false>>>& gpuParams)
		:mParamIndex(0), mMaterialParams(nullptr), mGPUParams(gpuParams)
	{
		if (params != nullptr)
		{
			const MaterialParams::ParamData* data = nullptr;
			auto result = params->getParamData(name, MaterialParams::ParamType::Texture, GPDT_UNKNOWN, 0, &data);

			if (result == MaterialParams::GetParamResult::Success)
			{
				mMaterialParams = params;
				mParamIndex = data->index;
			}
			else
				params->reportGetParamError(result, name, 0);
		}
	}

	void TMaterialParamTexture<false>::set(const HTexture& texture)
	{
		if (mMaterialParams == nullptr)
			return;

		mMaterialParams->setTexture(mParamIndex, texture);

		if (mGPUParams != nullptr)
		{
			for (auto& param : *mGPUParams)
				param.set(texture);
		}
	}

	HTexture TMaterialParamTexture<false>::get()
	{
		HTexture texture;
		if (mMaterialParams == nullptr)
			return texture;

		mMaterialParams->getTexture(mParamIndex, texture);

		return texture;
	}

	TMaterialParamTexture<true>::TMaterialParamTexture(const SPtr<Vector<TGpuParamTexture<true>>>& params)
		:mParams(params)
	{ }

	void TMaterialParamTexture<true>::set(const SPtr<TextureCore>& texture)
	{
		if (mParams == nullptr)
			return;

		for (auto& param : *mParams)
			param.set(texture);
	}

	SPtr<TextureCore> TMaterialParamTexture<true>::get()
	{
		if (mParams == nullptr || mParams->size() == 0)
			return SPtr<TextureCore>();

		return (*mParams)[0].get(); // They should all have the same value
	}

	TMaterialParamLoadStoreTexture<false>::TMaterialParamLoadStoreTexture(const String& name, const SPtr<MaterialParams>& params, 
		const SPtr<Vector<TGpuParamLoadStoreTexture<false>>>& gpuParams)
		:mParamIndex(0), mMaterialParams(nullptr), mGPUParams(gpuParams)
	{
		if (params != nullptr)
		{
			const MaterialParams::ParamData* data = nullptr;
			auto result = params->getParamData(name, MaterialParams::ParamType::Texture, GPDT_UNKNOWN, 0, &data);

			if (result == MaterialParams::GetParamResult::Success)
			{
				mMaterialParams = params;
				mParamIndex = data->index;
			}
			else
				params->reportGetParamError(result, name, 0);
		}
	}

	void TMaterialParamLoadStoreTexture<false>::set(const HTexture& texture, const TextureSurface& surface)
	{
		if (mMaterialParams == nullptr)
			return;

		mMaterialParams->setLoadStoreTexture(mParamIndex, texture, surface);

		if (mGPUParams != nullptr)
		{
			for (auto& param : *mGPUParams)
				param.set(texture, surface);
		}
	}

	HTexture TMaterialParamLoadStoreTexture<false>::get()
	{
		HTexture texture;
		if (mMaterialParams == nullptr)
			return texture;

		TextureSurface surface;
		mMaterialParams->getLoadStoreTexture(mParamIndex, texture, surface);

		return texture;
	}

	TMaterialParamLoadStoreTexture<true>::TMaterialParamLoadStoreTexture(const SPtr<Vector<TGpuParamLoadStoreTexture<true>>>& params)
		:mParams(params)
	{ }

	void TMaterialParamLoadStoreTexture<true>::set(const SPtr<TextureCore>& texture, const TextureSurface& surface)
	{
		if (mParams == nullptr)
			return;

		for (auto& param : *mParams)
			param.set(texture, surface);
	}

	SPtr<TextureCore> TMaterialParamLoadStoreTexture<true>::get()
	{
		if (mParams == nullptr || mParams->size() == 0)
			return SPtr<TextureCore>();

		return (*mParams)[0].get(); // They should all have the same value
	}

	TMaterialParamSampState<false>::TMaterialParamSampState(const String& name, const SPtr<MaterialParams>& params, 
		const SPtr<Vector<TGpuParamSampState<false>>>& gpuParams)
		:mParamIndex(0), mMaterialParams(nullptr), mGPUParams(gpuParams)
	{
		if (params != nullptr)
		{
			const MaterialParams::ParamData* data = nullptr;
			auto result = params->getParamData(name, MaterialParams::ParamType::Sampler, GPDT_UNKNOWN, 0, &data);

			if (result == MaterialParams::GetParamResult::Success)
			{
				mMaterialParams = params;
				mParamIndex = data->index;
			}
			else
				params->reportGetParamError(result, name, 0);
		}
	}

	void TMaterialParamSampState<false>::set(const SPtr<SamplerState>& sampState)
	{
		if (mMaterialParams == nullptr)
			return;

		mMaterialParams->setSamplerState(mParamIndex, sampState);

		if (mGPUParams != nullptr)
		{
			for (auto& param : *mGPUParams)
				param.set(sampState);
		}
	}

	SPtr<SamplerState> TMaterialParamSampState<false>::get()
	{
		SPtr<SamplerState> samplerState;
		if (mMaterialParams == nullptr)
			return samplerState;

		mMaterialParams->getSamplerState(mParamIndex, samplerState);
		return samplerState;
	}

	TMaterialParamSampState<true>::TMaterialParamSampState(const SPtr<Vector<TGpuParamSampState<true>>>& params)
		:mParams(params)
	{ }

	void TMaterialParamSampState<true>::set(const SPtr<SamplerStateCore>& sampState)
	{
		if (mParams == nullptr)
			return;

		for (auto& param : *mParams)
			param.set(sampState);
	}

	SPtr<SamplerStateCore> TMaterialParamSampState<true>::get()
	{
		if (mParams == nullptr || mParams->size() == 0)
			return SPtr<SamplerStateCore>();

		return (*mParams)[0].get(); // They should all have the same value
	}

	template class TMaterialDataParam<float, false>;
	template class TMaterialDataParam<int, false>;
	template class TMaterialDataParam<Color, false>;
	template class TMaterialDataParam<Vector2, false>;
	template class TMaterialDataParam<Vector3, false>;
	template class TMaterialDataParam<Vector4, false>;
	template class TMaterialDataParam<Vector2I, false>;
	template class TMaterialDataParam<Vector3I, false>;
	template class TMaterialDataParam<Vector4I, false>;
	template class TMaterialDataParam<Matrix2, false>;
	template class TMaterialDataParam<Matrix2x3, false>;
	template class TMaterialDataParam<Matrix2x4, false>;
	template class TMaterialDataParam<Matrix3, false>;
	template class TMaterialDataParam<Matrix3x2, false>;
	template class TMaterialDataParam<Matrix3x4, false>;
	template class TMaterialDataParam<Matrix4, false>;
	template class TMaterialDataParam<Matrix4x2, false>;
	template class TMaterialDataParam<Matrix4x3, false>;

	template class TMaterialDataParam<float, true>;
	template class TMaterialDataParam<int, true>;
	template class TMaterialDataParam<Color, true>;
	template class TMaterialDataParam<Vector2, true>;
	template class TMaterialDataParam<Vector3, true>;
	template class TMaterialDataParam<Vector4, true>;
	template class TMaterialDataParam<Vector2I, true>;
	template class TMaterialDataParam<Vector3I, true>;
	template class TMaterialDataParam<Vector4I, true>;
	template class TMaterialDataParam<Matrix2, true>;
	template class TMaterialDataParam<Matrix2x3, true>;
	template class TMaterialDataParam<Matrix2x4, true>;
	template class TMaterialDataParam<Matrix3, true>;
	template class TMaterialDataParam<Matrix3x2, true>;
	template class TMaterialDataParam<Matrix3x4, true>;
	template class TMaterialDataParam<Matrix4, true>;
	template class TMaterialDataParam<Matrix4x2, true>;
	template class TMaterialDataParam<Matrix4x3, true>;

	template class TMaterialParamStruct<false>;
	template class TMaterialParamStruct<true>;

	template class TMaterialParamTexture<false>;
	template class TMaterialParamTexture<true>;

	template class TMaterialParamLoadStoreTexture<false>;
	template class TMaterialParamLoadStoreTexture<true>;

	template class TMaterialParamSampState<false>;
	template class TMaterialParamSampState<true>;
}