//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsIReflectable.h"
#include "BsStaticAlloc.h"
#include "BsVector2.h"
#include "BsVector3.h"
#include "BsVector4.h"
#include "BsVector2I.h"
#include "BsVectorNI.h"
#include "BsColor.h"
#include "BsMatrix3.h"
#include "BsMatrix4.h"
#include "BsMatrixNxM.h"
#include "BsGpuParams.h"

namespace BansheeEngine
{
	/** @addtogroup Material-Internal
	 *  @{
	 */

	/** 
	 * Contains all parameter values set in a Material. This is similar to GpuParams which also stores parameter values,
	 * however GpuParams are built for use on the GPU-side and don't store parameters that don't exist in a compiled GPU
	 * program. This object on the other hand stores all parameters defined in a shader, regardless or not if they actually
	 * exist in the GPU program. Additionally GpuParams are defined per-program (for example vertex, fragment) while this
	 * object exists for the entire material.
	 *
	 * @note
	 * This introduces redundancy as parameters stored by GpuParams and this object are duplicated. If this is an issue the
	 * implementation can be modified to only store parameters not included in GpuParams.
	 * @note
	 * The reason why parameters in this class and GpuParams differ is most often compiler optimizations. If a compiler
	 * optimizes out a variable in a GPU program we should still be able to store it, either for later when the variable
	 * will be introduced, or for other techniques that might have that variable implemented.
	 */
	class BS_CORE_EXPORT MaterialParams : public IReflectable
	{
	public:
		/** Type of material parameter. */
		enum class ParamType
		{
			Data, Texture, Sampler
		};

		/** Result codes for getParam method. */
		enum class GetParamResult
		{
			Success,
			NotFound,
			InvalidType,
			IndexOutOfBounds
		};

		/** Meta-data about a parameter. */
		struct ParamData
		{
			ParamType type;
			GpuParamDataType dataType;
			UINT32 index;
			UINT32 arraySize;
		};

		/** Raw data for a single structure parameter. */
		class BS_CORE_EXPORT StructParamData : public IReflectable
		{
		public:
			UINT8* data;
			UINT32 dataSize;

			friend class StructParamDataRTTI;
			static RTTITypeBase* getRTTIStatic();
			virtual RTTITypeBase* getRTTI() const override;
		};

		/** Data for a single texture parameter. */
		class BS_CORE_EXPORT TextureParamData : public IReflectable
		{
		public:
			HTexture value;
			bool isLoadStore;
			TextureSurface surface;

			friend class TextureParamDataRTTI;
			static RTTITypeBase* getRTTIStatic();
			virtual RTTITypeBase* getRTTI() const override;
		};

		/** Creates a new material params object and initializes enough room for parameters from the provided shader. */
		MaterialParams(const HShader& shader);
		~MaterialParams();

		/** 
		 * Returns the value of a shader data parameter with the specified name at the specified array index. If the
		 * parameter name, index or type is not valid a warning will be logged and output value will not be retrieved.
		 * 
		 * @param[in]	name		Name of the shader parameter.
		 * @param[in]	arrayIdx	If the parameter is an array, index of the entry to access.
		 * @param[out]	output		If successful, value of the parameter.
		 *
		 * @tparam		T			Native type of the parameter.
		 */
		template <typename T>
		void getDataParam(const String& name, UINT32 arrayIdx, T& output) const
		{
			GpuParamDataType dataType = TGpuDataParamInfo<T>::TypeId;

			const ParamData* param = nullptr;
			auto result = getParamData(name, ParamType::Data, dataType, arrayIdx, &param);
			if (result != GetParamResult::Success)
				return;

			const GpuParamDataTypeInfo& typeInfo = GpuParams::PARAM_SIZES[dataType];
			UINT32 paramTypeSize = typeInfo.numColumns * typeInfo.numRows * typeInfo.baseTypeSize;
			output = *(T*)(mDataParamsBuffer[param->index + arrayIdx * paramTypeSize]);

			memcpy(output, &mDataParamsBuffer[param->index + arrayIdx * paramTypeSize], sizeof(paramTypeSize));
		}

		/** 
		 * Sets the value of a shader data parameter with the specified name at the specified array index. If the
		 * parameter name, index or type is not valid a warning will be logged and output value will not be set.
		 * 
		 * @param[in]	name		Name of the shader parameter.
		 * @param[in]	arrayIdx	If the parameter is an array, index of the entry to access.
		 * @param[in]	input		New value of the parameter.
		 *
		 * @tparam		T			Native type of the parameter.
		 */
		template <typename T>
		void setDataParam(const String& name, UINT32 arrayIdx, const T& input) const
		{
			GpuParamDataType dataType = TGpuDataParamInfo<T>::TypeId;

			const ParamData* param = nullptr;
			auto result = getParamData(name, ParamType::Data, dataType, arrayIdx, &param);
			if (result != GetParamResult::Success)
				return;

			const GpuParamDataTypeInfo& typeInfo = GpuParams::PARAM_SIZES[dataType];
			UINT32 paramTypeSize = typeInfo.numColumns * typeInfo.numRows * typeInfo.baseTypeSize;

			memcpy(&mDataParamsBuffer[param->index + arrayIdx * paramTypeSize], input, sizeof(paramTypeSize));
		}

		/** 
		 * Returns the value of a shader structure parameter with the specified name at the specified array index. If the
		 * parameter name, index or type is not valid a warning will be logged and output value will not be retrieved.
		 * 
		 * @param[in]	name		Name of the shader parameter.
		 * @param[out]	value		Pre-allocated buffer of @p size bytes where the value will be retrieved.
		 * @param[in]	size		Size of the buffer into which to write the value. Must match parameter struct's size.
		 * @param[in]	arrayIdx	If the parameter is an array, index of the entry to access.
		 */
		void getStructData(const String& name, void* value, UINT32 size, UINT32 arrayIdx) const;

		/** 
		 * Sets the value of a shader structure parameter with the specified name at the specified array index. If the
		 * parameter name, index or type is not valid a warning will be logged and output value will not be retrieved.
		 * 
		 * @param[in]	name		Name of the shader parameter.
		 * @param[in]	value		Buffer of @p size bytes containing the new value of the structure.
		 * @param[in]	size		Size of the buffer from which to retrieve the value. Must match parameter struct's size.
		 * @param[in]	arrayIdx	If the parameter is an array, index of the entry to access.
		 */
		void setStructData(const String& name, const void* value, UINT32 size, UINT32 arrayIdx);

		/** 
		 * Returns the value of a shader texture parameter with the specified name. If the parameter name or type is not 
		 * valid a warning will be logged and output value will not be retrieved.
		 * 
		 * @param[in]	name		Name of the shader parameter.
		 * @param[out]	value		Output value of the parameter.
		 */
		void getTexture(const String& name, HTexture& value) const;

		/** 
		 * Sets the value of a shader texture parameter with the specified name. If the parameter name or type is not 
		 * valid a warning will be logged and output value will not be set.
		 * 
		 * @param[in]	name		Name of the shader parameter.
		 * @param[in]	value		New value of the parameter.
		 */
		void setTexture(const String& name, const HTexture& value);

		/** 
		 * Returns the value of a shader load/store texture parameter with the specified name. If the parameter name or 
		 * type is not valid a warning will be logged and output value will not be retrieved.
		 * 
		 * @param[in]	name		Name of the shader parameter.
		 * @param[out]	value		Output value of the parameter.
		 * @param[out]	surface		Surface describing which part of the texture is being accessed.
		 */
		void getLoadStoreTexture(const String& name, HTexture& value, TextureSurface& surface) const;

		/** 
		 * Sets the value of a shader load/store texture parameter with the specified name. If the parameter name or 
		 * type is not valid a warning will be logged and the value will not be set.
		 * 
		 * @param[in]	name		Name of the shader parameter.
		 * @param[in]	value		New value of the parameter.
		 * @param[in]	surface		Surface describing which part of the texture is being accessed.
		 */
		void setLoadStoreTexture(const String& name, const HTexture& value, const TextureSurface& surface);

		/** 
		 * Sets the value of a shader sampler state parameter with the specified name. If the parameter name or type is not 
		 * valid a warning will be logged and output value will not be set.
		 * 
		 * @param[in]	name		Name of the shader parameter.
		 * @param[out]	value		Output value of the parameter.
		 */
		void getSamplerState(const String& name, SamplerStatePtr& value) const;

		/** 
		 * Sets the value of a shader sampler state parameter with the specified name. If the parameter name or type is not 
		 * valid a warning will be logged and output value will not be set.
		 * 
		 * @param[in]	name		Name of the shader parameter.
		 * @param[in]	value		New value of the parameter.
		 */
		void setSamplerState(const String& name, const SamplerStatePtr& value);

		/** 
		 * Returns data about a parameter and reports an error if there is a type or size mismatch, or if the parameter
		 * does exist.
		 *
		 * @param[in]	name		Name of the shader parameter.
		 * @param[in]	type		Type of the parameter retrieve. Error will be logged if actual type of the parameter 
		 *							doesn't match.
		 * @param[in]	dataType	Only relevant if the parameter is a data type. Determines exact data type of the parameter
		 *							to retrieve. 
		 * @param[in]	arrayIdx	Array index of the entry to retrieve. 
		 * @param[out]	output		Object describing the parameter with an index to its data. If the parameter was not found
		 *							this value is undefined. This value will still be valid if parameter was found but
		 *							some other error was reported.
		 *
		 * @return					Success or error state of the request.
		 */
		GetParamResult getParamData(const String& name, ParamType type, GpuParamDataType dataType, UINT32 arrayIdx, 
			const ParamData** output) const;

		/**
		 * Logs an error that was reported by getParamData().
		 *
		 * @param[in]	errorCode	Information about the error.
		 * @param[in]	name		Name of the shader parameter for which the error occurred.
		 * @param[in]	arrayIdx	Array index for which the error occurred. 
		 */
		void reportGetParamError(GetParamResult errorCode, const String& name, UINT32 arrayIdx) const;

		/** 
		 * Equivalent to getStructData(const String&, UINT32, T&) except it uses the internal parameter index
		 * directly, avoiding the name lookup. Caller must guarantee the index is valid.
		 */
		template <typename T>
		void getDataParam(UINT32 index, UINT32 arrayIdx, T& output) const
		{
			GpuParamDataType dataType = (GpuParamDataType)TGpuDataParamInfo<T>::TypeId;

			const GpuParamDataTypeInfo& typeInfo = GpuParams::PARAM_SIZES.lookup[dataType];
			UINT32 paramTypeSize = typeInfo.numColumns * typeInfo.numRows * typeInfo.baseTypeSize;

			assert(sizeof(output) == paramTypeSize);
			memcpy(&output, &mDataParamsBuffer[index + arrayIdx * paramTypeSize], paramTypeSize);
		}

		/** 
		 * Equivalent to setDataParam(const String&, UINT32, T&) except it uses the internal parameter index
		 * directly, avoiding the name lookup. Caller must guarantee the index is valid.
		 */
		template <typename T>
		void setDataParam(UINT32 index, UINT32 arrayIdx, const T& input) const
		{
			GpuParamDataType dataType = (GpuParamDataType)TGpuDataParamInfo<T>::TypeId;

			const GpuParamDataTypeInfo& typeInfo = GpuParams::PARAM_SIZES.lookup[dataType];
			UINT32 paramTypeSize = typeInfo.numColumns * typeInfo.numRows * typeInfo.baseTypeSize;

			assert(sizeof(input) == paramTypeSize);
			memcpy(&mDataParamsBuffer[index + arrayIdx * paramTypeSize], &input, paramTypeSize);
		}

		/** 
		 * Equivalent to getStructData(const String&, UINT32, void*, UINT32) except it uses the internal parameter index
		 * directly, avoiding the name lookup. Caller must guarantee the index is valid.
		 */
		void getStructData(UINT32 index, void* value, UINT32 size) const;

		/** 
		 * Equivalent to setStructData(const String&, UINT32, void*, UINT32) except it uses the internal parameter index
		 * directly, avoiding the name lookup. Caller must guarantee the index is valid.
		 */
		void setStructData(UINT32 index, const void* value, UINT32 size);

		/** 
		 * Returns a size of a struct parameter in bytes, using the internal parameter index. Caller must guarantee the 
		 * index is valid. 
		 */
		UINT32 getStructSize(UINT32 index) const;

		/** 
		 * Equivalent to getTexture(const String&, HTexture&) except it uses the internal parameter index directly, 
		 * avoiding the name lookup. Caller must guarantee the index is valid.
		 */
		void getTexture(UINT32 index, HTexture& value) const;

		/** 
		 * Equivalent to setTexture(const String&, HTexture&) except it uses the internal parameter index directly, 
		 * avoiding the name lookup. Caller must guarantee the index is valid.
		 */
		void setTexture(UINT32 index, const HTexture& value);

		/** 
		 * Equivalent to getLoadStoreTexture(const String&, HTexture&, TextureSurface&) except it uses the internal 
		 * parameter index directly, avoiding the name lookup. Caller must guarantee the index is valid.
		 */
		void getLoadStoreTexture(UINT32 index, HTexture& value, TextureSurface& surface) const;

		/** 
		 * Equivalent to setLoadStoreTexture(const String&, HTexture&, TextureSurface&) except it uses the internal 
		 * parameter index directly, avoiding the name lookup. Caller must guarantee the index is valid.
		 */
		void setLoadStoreTexture(UINT32 index, const HTexture& value, const TextureSurface& surface);

		/** 
		 * Checks is a texture with the specified index a load/store texture or a normal one. Caller must guarantee the 
		 * index is valid.
		 */
		bool getIsTextureLoadStore(UINT32 index) const;

		/** 
		 * Equivalent to getSamplerState(const String&, SamplerStatePtr&) except it uses the internal parameter index 
		 * directly, avoiding the name lookup. Caller must guarantee the index is valid.
		 */
		void getSamplerState(UINT32 index, SamplerStatePtr& value) const;

		/** 
		 * Equivalent to setSamplerState(const String&, SamplerStatePtr&) except it uses the internal parameter index 
		 * directly, avoiding the name lookup. Caller must guarantee the index is valid.
		 */
		void setSamplerState(UINT32 index, const SamplerStatePtr& value);

		/** 
		 * Returns the default texture (one assigned when no other is provided), if available for the specified index. 
		 * Index is the internal parameter index and the caller must guarantee the index is valid.
		 */
		void getDefaultTexture(UINT32 index, HTexture& value) const;

		/** 
		 * Returns the default sampler state (one assigned when no other is provided), if available for the specified index.
		 * Index is the internal parameter index and the caller must guarantee the index is valid.
		 */
		void getDefaultSamplerState(UINT32 index, SamplerStatePtr& value) const;

	private:
		const static UINT32 STATIC_BUFFER_SIZE = 256;

		UnorderedMap<String, ParamData> mParams;

		UINT8* mDataParamsBuffer = nullptr;
		StructParamData* mStructParams = nullptr;
		TextureParamData* mTextureParams = nullptr;
		SamplerStatePtr* mSamplerStateParams = nullptr;
		HTexture* mDefaultTextureParams = nullptr;
		SamplerStatePtr* mDefaultSamplerStateParams = nullptr;

		UINT32 mDataSize = 0;
		UINT32 mNumStructParams = 0;
		UINT32 mNumTextureParams = 0;
		UINT32 mNumSamplerParams = 0;

		mutable StaticAlloc<STATIC_BUFFER_SIZE, STATIC_BUFFER_SIZE> mAlloc;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		MaterialParams() { } // Only for serialization

		friend class MaterialParamsRTTI;
		static RTTITypeBase* getRTTIStatic();
		virtual RTTITypeBase* getRTTI() const override;
	};

	/** @cond SPECIALIZATIONS */
	BS_ALLOW_MEMCPY_SERIALIZATION(MaterialParams::ParamData);
	/** @endcond */

	/** @} */
}
