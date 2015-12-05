#include "BsMaterial.h"
#include "BsException.h"
#include "BsShader.h"
#include "BsTechnique.h"
#include "BsPass.h"
#include "BsRenderAPI.h"
#include "BsHardwareBufferManager.h"
#include "BsGpuProgram.h"
#include "BsGpuParamBlockBuffer.h"
#include "BsGpuParamDesc.h"
#include "BsMaterialRTTI.h"
#include "BsMaterialManager.h"
#include "BsDebug.h"
#include "BsResources.h"
#include "BsFrameAlloc.h"
#include "BsMatrixNxM.h"
#include "BsVectorNI.h"
#include "BsMemorySerializer.h"

namespace BansheeEngine
{
	struct ShaderBlockDesc
	{
		String name;
		GpuParamBlockUsage usage;
		int size;
		bool create;
	};

	enum MaterialLoadFlags
	{
		Load_None	= 0,
		Load_Shader	= 1,
		Load_All	= 2
	};

	const UINT32 PassParameters::NUM_PARAMS = 6;
	const UINT32 PassParametersCore::NUM_PARAMS = 6;
	
	SPtr<PassParametersCore> convertParamsToCore(const SPtr<PassParameters>& passParams)
	{
		SPtr<PassParametersCore> passParameters = bs_shared_ptr_new<PassParametersCore>();

		if (passParams->mVertParams != nullptr)
			passParameters->mVertParams = passParams->mVertParams->getCore();
		else
			passParameters->mVertParams = nullptr;

		if (passParams->mFragParams != nullptr)
			passParameters->mFragParams = passParams->mFragParams->getCore();
		else
			passParameters->mFragParams = nullptr;

		if (passParams->mGeomParams != nullptr)
			passParameters->mGeomParams = passParams->mGeomParams->getCore();
		else
			passParameters->mGeomParams = nullptr;

		if (passParams->mHullParams != nullptr)
			passParameters->mHullParams = passParams->mHullParams->getCore();
		else
			passParameters->mHullParams = nullptr;

		if (passParams->mDomainParams != nullptr)
			passParameters->mDomainParams = passParams->mDomainParams->getCore();
		else
			passParameters->mDomainParams = nullptr;

		if (passParams->mComputeParams != nullptr)
			passParameters->mComputeParams = passParams->mComputeParams->getCore();
		else
			passParameters->mComputeParams = nullptr;

		return passParameters;
	}

	bool areParamsEqual(const GpuParamDataDesc& paramA, const GpuParamDataDesc& paramB, bool ignoreBufferOffsets)
	{
		bool equal = paramA.arraySize == paramB.arraySize && paramA.elementSize == paramB.elementSize
			&& paramA.type == paramB.type && paramA.arrayElementStride == paramB.arrayElementStride;

		if (!ignoreBufferOffsets)
			equal &= paramA.cpuMemOffset == paramB.cpuMemOffset && paramA.gpuMemOffset == paramB.gpuMemOffset;

		return equal;
	}

	Map<String, const GpuParamDataDesc*> determineValidDataParameters(const Vector<GpuParamDescPtr>& paramDescs)
	{
		Map<String, const GpuParamDataDesc*> foundDataParams;
		Map<String, bool> validParams;

		for (auto iter = paramDescs.begin(); iter != paramDescs.end(); ++iter)
		{
			const GpuParamDesc& curDesc = **iter;

			// Check regular data params
			for (auto iter2 = curDesc.params.begin(); iter2 != curDesc.params.end(); ++iter2)
			{
				bool isParameterValid = true;
				const GpuParamDataDesc& curParam = iter2->second;

				auto dataFindIter = validParams.find(iter2->first);
				if (dataFindIter == validParams.end())
				{
					validParams[iter2->first] = true;
					foundDataParams[iter2->first] = &curParam;
				}
				else
				{
					if (validParams[iter2->first])
					{
						auto dataFindIter2 = foundDataParams.find(iter2->first);

						const GpuParamDataDesc* otherParam = dataFindIter2->second;
						if (!areParamsEqual(curParam, *otherParam, true))
						{
							validParams[iter2->first] = false;
							foundDataParams.erase(dataFindIter2);
						}
					}
				}
			}
		}

		return foundDataParams;
	}

	Vector<const GpuParamObjectDesc*> determineValidObjectParameters(const Vector<GpuParamDescPtr>& paramDescs)
	{
		Vector<const GpuParamObjectDesc*> validParams;

		for (auto iter = paramDescs.begin(); iter != paramDescs.end(); ++iter)
		{
			const GpuParamDesc& curDesc = **iter;

			// Check sampler params
			for (auto iter2 = curDesc.samplers.begin(); iter2 != curDesc.samplers.end(); ++iter2)
			{
				validParams.push_back(&iter2->second);
			}

			// Check texture params
			for (auto iter2 = curDesc.textures.begin(); iter2 != curDesc.textures.end(); ++iter2)
			{
				validParams.push_back(&iter2->second);
			}

			// Check buffer params
			for (auto iter2 = curDesc.buffers.begin(); iter2 != curDesc.buffers.end(); ++iter2)
			{
				validParams.push_back(&iter2->second);
			}
		}

		return validParams;
	}

	Set<String> determineValidShareableParamBlocks(const Vector<GpuParamDescPtr>& paramDescs)
	{
		// Make sure param blocks with the same name actually are the same
		Map<String, std::pair<String, GpuParamDescPtr>> uniqueParamBlocks;
		Map<String, bool> validParamBlocks;

		for (auto iter = paramDescs.begin(); iter != paramDescs.end(); ++iter)
		{
			const GpuParamDesc& curDesc = **iter;
			for (auto blockIter = curDesc.paramBlocks.begin(); blockIter != curDesc.paramBlocks.end(); ++blockIter)
			{
				bool isBlockValid = true;
				const GpuParamBlockDesc& curBlock = blockIter->second;

				if (!curBlock.isShareable) // Non-shareable buffers are handled differently, they're allowed same names
					continue;

				auto iterFind = uniqueParamBlocks.find(blockIter->first);
				if (iterFind == uniqueParamBlocks.end())
				{
					uniqueParamBlocks[blockIter->first] = std::make_pair(blockIter->first, *iter);
					validParamBlocks[blockIter->first] = true;
					continue;
				}

				String otherBlockName = iterFind->second.first;
				GpuParamDescPtr otherDesc = iterFind->second.second;

				for (auto myParamIter = curDesc.params.begin(); myParamIter != curDesc.params.end(); ++myParamIter)
				{
					const GpuParamDataDesc& myParam = myParamIter->second;

					if (myParam.paramBlockSlot != curBlock.slot)
						continue; // Param is in another block, so we will check it when its time for that block

					auto otherParamFind = otherDesc->params.find(myParamIter->first);

					// Cannot find other param, blocks aren't equal
					if (otherParamFind == otherDesc->params.end())
					{
						isBlockValid = false;
						break;
					}

					const GpuParamDataDesc& otherParam = otherParamFind->second;

					if (!areParamsEqual(myParam, otherParam, false) || curBlock.name != otherBlockName)
					{
						isBlockValid = false;
						break;
					}
				}

				if (!isBlockValid)
				{
					if (validParamBlocks[blockIter->first])
					{
						LOGWRN("Found two param blocks with the same name but different contents: " + blockIter->first);
						validParamBlocks[blockIter->first] = false;
					}
				}
			}
		}

		Set<String> validParamBlocksReturn;
		for (auto iter = validParamBlocks.begin(); iter != validParamBlocks.end(); ++iter)
		{
			if (iter->second)
				validParamBlocksReturn.insert(iter->first);
		}

		return validParamBlocksReturn;
	}

	Map<String, String> determineParameterToBlockMapping(const Vector<GpuParamDescPtr>& paramDescs)
	{
		Map<String, String> paramToParamBlock;

		for (auto iter = paramDescs.begin(); iter != paramDescs.end(); ++iter)
		{
			const GpuParamDesc& curDesc = **iter;
			for (auto iter2 = curDesc.params.begin(); iter2 != curDesc.params.end(); ++iter2)
			{
				const GpuParamDataDesc& curParam = iter2->second;

				auto iterFind = paramToParamBlock.find(curParam.name);
				if (iterFind != paramToParamBlock.end())
					continue;

				for (auto iterBlock = curDesc.paramBlocks.begin(); iterBlock != curDesc.paramBlocks.end(); ++iterBlock)
				{
					if (iterBlock->second.slot == curParam.paramBlockSlot)
					{
						paramToParamBlock[curParam.name] = iterBlock->second.name;
						break;
					}
				}
			}
		}

		return paramToParamBlock;
	}

	Map<String, String> determineParamMappings(const Vector<GpuParamDescPtr>& paramDescs, 
		const Map<String, SHADER_DATA_PARAM_DESC>& dataParams,
		const Map<String, SHADER_OBJECT_PARAM_DESC>& textureParams,
		const Map<String, SHADER_OBJECT_PARAM_DESC>& bufferParams,
		const Map<String, SHADER_OBJECT_PARAM_DESC>& samplerParams)
	{
		Map<String, String> validParams;

		Map<String, const GpuParamDataDesc*> validDataParameters = determineValidDataParameters(paramDescs);
		Vector<const GpuParamObjectDesc*> validObjectParameters = determineValidObjectParameters(paramDescs);
		Map<String, String> paramToParamBlockMap = determineParameterToBlockMapping(paramDescs);

		// Create data param mappings
		for (auto iter = dataParams.begin(); iter != dataParams.end(); ++iter)
		{
			auto findIter = validDataParameters.find(iter->second.gpuVariableName);

			// Not valid so we skip it
			if (findIter == validDataParameters.end())
				continue;

			if (findIter->second->type != iter->second.type && !(iter->second.type == GPDT_COLOR && findIter->second->type == GPDT_FLOAT4))
			{
				LOGWRN("Ignoring shader parameter \"" + iter->first + "\". Type doesn't match the one defined in the gpu program. "
					+ "Shader defined type: " + toString(iter->second.type) + " - Gpu program defined type: " + toString(findIter->second->type));
				continue;
			}

			if (findIter->second->arraySize != iter->second.arraySize)
			{
				LOGWRN("Ignoring shader parameter \"" + iter->first + "\". Array size doesn't match the one defined in the gpu program."
					+ "Shader defined array size: " + toString(iter->second.arraySize) + " - Gpu program defined array size: " + toString(findIter->second->arraySize));
				continue;
			}

			auto findBlockIter = paramToParamBlockMap.find(iter->second.gpuVariableName);

			if (findBlockIter == paramToParamBlockMap.end())
				BS_EXCEPT(InternalErrorException, "Parameter doesn't exist in param to param block map but exists in valid param map.");

			validParams[iter->first] = iter->second.gpuVariableName;
		}

		// Create object param mappings
		auto determineObjectMappings = [&](const Map<String, SHADER_OBJECT_PARAM_DESC>& params)
		{
			for (auto iter = params.begin(); iter != params.end(); ++iter)
			{
				const Vector<String>& gpuVariableNames = iter->second.gpuVariableNames;
				for (auto iter2 = gpuVariableNames.begin(); iter2 != gpuVariableNames.end(); ++iter2)
				{
					for (auto iter3 = validObjectParameters.begin(); iter3 != validObjectParameters.end(); ++iter3)
					{
						if ((*iter3)->name == (*iter2) && (*iter3)->type == iter->second.type)
						{
							validParams[iter->first] = *iter2;
							break;
						}
					}
				}
			}
		};

		determineObjectMappings(textureParams);
		determineObjectMappings(samplerParams);
		determineObjectMappings(bufferParams);

		return validParams;
	}

	Vector<ShaderBlockDesc> determineShaderBlockData(const Set<String>& paramBlocks, const Vector<GpuParamDescPtr>& paramDescs,
		const Map<String, SHADER_PARAM_BLOCK_DESC>& shaderDesc)
	{
		Vector<ShaderBlockDesc> output;
		for (auto iter = paramBlocks.begin(); iter != paramBlocks.end(); ++iter)
		{
			ShaderBlockDesc shaderBlockDesc;
			shaderBlockDesc.create = true;
			shaderBlockDesc.usage = GPBU_STATIC;
			shaderBlockDesc.size = 0;
			shaderBlockDesc.name = *iter;

			auto iterFind = shaderDesc.find(*iter);
			if (iterFind != shaderDesc.end())
			{
				shaderBlockDesc.create = !iterFind->second.shared && iterFind->second.rendererSemantic == StringID::NONE;
				shaderBlockDesc.usage = iterFind->second.usage;
			}

			for (auto iter2 = paramDescs.begin(); iter2 != paramDescs.end(); ++iter2)
			{
				auto findParamBlockDesc = (*iter2)->paramBlocks.find(*iter);

				if (findParamBlockDesc != (*iter2)->paramBlocks.end())
				{
					shaderBlockDesc.size = findParamBlockDesc->second.blockSize * sizeof(UINT32);
					break;
				}
			}

			output.push_back(shaderBlockDesc);
		}

		return output;
	}

	Vector<GpuParamDescPtr> MaterialBase::getAllParamDescs(const SPtr<Technique>& technique)
	{
		Vector<GpuParamDescPtr> allParamDescs;

		// Make sure all gpu programs are fully loaded
		for (UINT32 i = 0; i < technique->getNumPasses(); i++)
		{
			PassPtr curPass = technique->getPass(i);

			GpuProgramPtr vertProgram = curPass->getVertexProgram();
			if (vertProgram)
			{
				vertProgram->blockUntilCoreInitialized();
				allParamDescs.push_back(vertProgram->getParamDesc());
			}

			GpuProgramPtr fragProgram = curPass->getFragmentProgram();
			if (fragProgram)
			{
				fragProgram->blockUntilCoreInitialized();
				allParamDescs.push_back(fragProgram->getParamDesc());
			}

			GpuProgramPtr geomProgram = curPass->getGeometryProgram();
			if (geomProgram)
			{
				geomProgram->blockUntilCoreInitialized();
				allParamDescs.push_back(geomProgram->getParamDesc());
			}

			GpuProgramPtr hullProgram = curPass->getHullProgram();
			if (hullProgram)
			{
				hullProgram->blockUntilCoreInitialized();
				allParamDescs.push_back(hullProgram->getParamDesc());
			}

			GpuProgramPtr domainProgram = curPass->getDomainProgram();
			if (domainProgram)
			{
				domainProgram->blockUntilCoreInitialized();
				allParamDescs.push_back(domainProgram->getParamDesc());
			}

			GpuProgramPtr computeProgram = curPass->getComputeProgram();
			if (computeProgram)
			{
				computeProgram->blockUntilCoreInitialized();
				allParamDescs.push_back(computeProgram->getParamDesc());
			}
		}

		return allParamDescs;
	}

	Vector<GpuParamDescPtr> MaterialBase::getAllParamDescs(const SPtr<TechniqueCore>& technique)
	{
		Vector<GpuParamDescPtr> allParamDescs;

		// Make sure all gpu programs are fully loaded
		for (UINT32 i = 0; i < technique->getNumPasses(); i++)
		{
			SPtr<PassCore> curPass = technique->getPass(i);

			SPtr<GpuProgramCore> vertProgram = curPass->getVertexProgram();
			if (vertProgram)
				allParamDescs.push_back(vertProgram->getParamDesc());

			SPtr<GpuProgramCore> fragProgram = curPass->getFragmentProgram();
			if (fragProgram)
				allParamDescs.push_back(fragProgram->getParamDesc());

			SPtr<GpuProgramCore> geomProgram = curPass->getGeometryProgram();
			if (geomProgram)
				allParamDescs.push_back(geomProgram->getParamDesc());

			SPtr<GpuProgramCore> hullProgram = curPass->getHullProgram();
			if (hullProgram)
				allParamDescs.push_back(hullProgram->getParamDesc());

			SPtr<GpuProgramCore> domainProgram = curPass->getDomainProgram();
			if (domainProgram)
				allParamDescs.push_back(domainProgram->getParamDesc());

			SPtr<GpuProgramCore> computeProgram = curPass->getComputeProgram();
			if (computeProgram)
				allParamDescs.push_back(computeProgram->getParamDesc());
		}

		return allParamDescs;
	}

	template<bool Core>
	UINT32 TMaterial<Core>::getNumPasses() const
	{
		if (mShader == nullptr)
			return 0;

		return mShader->getBestTechnique()->getNumPasses();
	}

	template<bool Core>
	typename TMaterial<Core>::PassType TMaterial<Core>::getPass(UINT32 passIdx) const
	{
		if (passIdx < 0 || passIdx >= mShader->getBestTechnique()->getNumPasses())
			BS_EXCEPT(InvalidParametersException, "Invalid pass index.");

		return mShader->getBestTechnique()->getPass(passIdx);
	}

	template<bool Core>
	TMaterialParamStruct<Core> TMaterial<Core>::getParamStruct(const String& name) const
	{
		throwIfNotInitialized();

		auto iterFind = mValidParams.find(name);
		if (iterFind == mValidParams.end())
		{
			LOGWRN("Material doesn't have a parameter named " + name);
			return TMaterialParamStruct<Core>();
		}

		const String& gpuVarName = iterFind->second;
		SPtr<Vector<TGpuParamStruct<Core>>> gpuParams = bs_shared_ptr_new<Vector<TGpuParamStruct<Core>>>();

		for (auto iter = mParametersPerPass.begin(); iter != mParametersPerPass.end(); ++iter)
		{
			SPtr<PassParamsType> params = *iter;

			for (UINT32 i = 0; i < PassParamsType::NUM_PARAMS; i++)
			{
				GpuParamsType& paramPtr = params->getParamByIdx(i);
				if (paramPtr)
				{
					if (paramPtr->hasParam(gpuVarName))
					{
						gpuParams->push_back(TGpuParamStruct<Core>());
						paramPtr->getStructParam(gpuVarName, gpuParams->back());
					}
				}
			}
		}

		return TMaterialParamStruct<Core>(gpuParams);
	}

	template<bool Core>
	TMaterialParamTexture<Core> TMaterial<Core>::getParamTexture(const String& name) const
	{
		throwIfNotInitialized();

		auto iterFind = mValidParams.find(name);
		if (iterFind == mValidParams.end())
		{
			LOGWRN("Material doesn't have a parameter named " + name);
			return TMaterialParamTexture<Core>();
		}

		const String& gpuVarName = iterFind->second;
		SPtr<Vector<TGpuParamTexture<Core>>> gpuParams = bs_shared_ptr_new<Vector<TGpuParamTexture<Core>>>();

		for (auto iter = mParametersPerPass.begin(); iter != mParametersPerPass.end(); ++iter)
		{
			SPtr<PassParamsType> params = *iter;

			for (UINT32 i = 0; i < PassParamsType::NUM_PARAMS; i++)
			{
				GpuParamsType& paramPtr = params->getParamByIdx(i);
				if (paramPtr)
				{
					if (paramPtr->hasTexture(gpuVarName))
					{
						gpuParams->push_back(TGpuParamTexture<Core>());
						paramPtr->getTextureParam(gpuVarName, gpuParams->back());
					}
				}
			}
		}

		return TMaterialParamTexture<Core>(gpuParams);
	}

	template<bool Core>
	TMaterialParamLoadStoreTexture<Core> TMaterial<Core>::getParamLoadStoreTexture(const String& name) const
	{
		throwIfNotInitialized();

		auto iterFind = mValidParams.find(name);
		if (iterFind == mValidParams.end())
		{
			LOGWRN("Material doesn't have a parameter named " + name);
			return TMaterialParamLoadStoreTexture<Core>();
		}

		const String& gpuVarName = iterFind->second;
		SPtr<Vector<TGpuParamLoadStoreTexture<Core>>> gpuParams = bs_shared_ptr_new<Vector<TGpuParamLoadStoreTexture<Core>>>();

		for (auto iter = mParametersPerPass.begin(); iter != mParametersPerPass.end(); ++iter)
		{
			SPtr<PassParamsType> params = *iter;

			for (UINT32 i = 0; i < PassParamsType::NUM_PARAMS; i++)
			{
				GpuParamsType& paramPtr = params->getParamByIdx(i);
				if (paramPtr)
				{
					if (paramPtr->hasTexture(gpuVarName))
					{
						gpuParams->push_back(TGpuParamLoadStoreTexture<Core>());
						paramPtr->getLoadStoreTextureParam(gpuVarName, gpuParams->back());
					}
				}
			}
		}

		return TMaterialParamLoadStoreTexture<Core>(gpuParams);
	}

	template<bool Core>
	TMaterialParamSampState<Core> TMaterial<Core>::getParamSamplerState(const String& name) const
	{
		throwIfNotInitialized();

		auto iterFind = mValidParams.find(name);
		if (iterFind == mValidParams.end())
		{
			LOGWRN("Material doesn't have a parameter named " + name);
			return TMaterialParamSampState<Core>();
		}

		const String& gpuVarName = iterFind->second;
		SPtr<Vector<TGpuParamSampState<Core>>> gpuParams = bs_shared_ptr_new<Vector<TGpuParamSampState<Core>>>();
		for (auto iter = mParametersPerPass.begin(); iter != mParametersPerPass.end(); ++iter)
		{
			SPtr<PassParamsType> params = *iter;

			for (UINT32 i = 0; i < PassParamsType::NUM_PARAMS; i++)
			{
				GpuParamsType& paramPtr = params->getParamByIdx(i);
				if (paramPtr)
				{
					if (paramPtr->hasSamplerState(gpuVarName))
					{
						gpuParams->push_back(TGpuParamSampState<Core>());
						paramPtr->getSamplerStateParam(gpuVarName, gpuParams->back());
					}
				}
			}
		}

		return TMaterialParamSampState<Core>(gpuParams);
	}

	template<bool Core>
	void TMaterial<Core>::setParamBlockBuffer(const String& name, const ParamBlockPtrType& paramBlock)
	{
		auto iterFind = mValidShareableParamBlocks.find(name);
		if (iterFind == mValidShareableParamBlocks.end())
		{
			LOGWRN("Material doesn't have a parameter block named " + name);
			return;
		}

		for (auto iter = mParametersPerPass.begin(); iter != mParametersPerPass.end(); ++iter)
		{
			SPtr<PassParamsType> params = *iter;

			for (UINT32 i = 0; i < PassParamsType::NUM_PARAMS; i++)
			{
				GpuParamsType& paramPtr = params->getParamByIdx(i);
				if (paramPtr)
				{
					if (paramPtr->hasParamBlock(name))
						paramPtr->setParamBlockBuffer(name, paramBlock);
				}
			}
		}
	}

	template<bool Core>
	void TMaterial<Core>::initBestTechnique()
	{
		mBestTechnique = nullptr;
		mParametersPerPass.clear();

		if (mShader)
		{
			mBestTechnique = mShader->getBestTechnique();

			if (mBestTechnique == nullptr)
				return;

			mValidShareableParamBlocks.clear();
			Vector<GpuParamDescPtr> allParamDescs = getAllParamDescs(mBestTechnique);

			mValidParams = determineParamMappings(allParamDescs, mShader->getDataParams(), mShader->getTextureParams(),
				mShader->getBufferParams(), mShader->getSamplerParams());

			// Fill out various helper structures
			Set<String> validShareableParamBlocks = determineValidShareableParamBlocks(allParamDescs);
			Vector<ShaderBlockDesc> paramBlockData = determineShaderBlockData(validShareableParamBlocks, allParamDescs, mShader->getParamBlocks());

			Map<String, ParamBlockPtrType> paramBlockBuffers;

			// Create param blocks
			for (auto& paramBlock : paramBlockData)
			{
				ParamBlockPtrType newParamBlockBuffer;
				if (paramBlock.create)
				{
					newParamBlockBuffer = ParamBlockType::create(paramBlock.size, paramBlock.usage);
				}

				paramBlockBuffers[paramBlock.name] = newParamBlockBuffer;
				mValidShareableParamBlocks.insert(paramBlock.name);
			}

			for (UINT32 i = 0; i < mBestTechnique->getNumPasses(); i++)
			{
				PassType curPass = mBestTechnique->getPass(i);
				SPtr<PassParamsType> params = SPtr<PassParamsType>(new PassParamsType());

				GpuProgramType vertProgram = curPass->getVertexProgram();
				if (vertProgram)
					params->mVertParams = vertProgram->createParameters();

				GpuProgramType fragProgram = curPass->getFragmentProgram();
				if (fragProgram)
					params->mFragParams = fragProgram->createParameters();

				GpuProgramType geomProgram = curPass->getGeometryProgram();
				if (geomProgram)
					params->mGeomParams = geomProgram->createParameters();

				GpuProgramType hullProgram = curPass->getHullProgram();
				if (hullProgram)
					params->mHullParams = hullProgram->createParameters();

				GpuProgramType domainProgram = curPass->getDomainProgram();
				if (domainProgram)
					params->mDomainParams = domainProgram->createParameters();

				GpuProgramType computeProgram = curPass->getComputeProgram();
				if (computeProgram)
					params->mComputeParams = computeProgram->createParameters();

				mParametersPerPass.push_back(params);
			}

			// Assign param block buffers
			for (auto iter = mParametersPerPass.begin(); iter != mParametersPerPass.end(); ++iter)
			{
				SPtr<PassParamsType> params = *iter;

				for (UINT32 i = 0; i < PassParamsType::NUM_PARAMS; i++)
				{
					GpuParamsType& paramPtr = params->getParamByIdx(i);
					if (paramPtr)
					{
						// Assign shareable buffers
						for (auto iterBlock = mValidShareableParamBlocks.begin(); iterBlock != mValidShareableParamBlocks.end(); ++iterBlock)
						{
							const String& paramBlockName = *iterBlock;
							if (paramPtr->hasParamBlock(paramBlockName))
							{
								ParamBlockPtrType blockBuffer = paramBlockBuffers[paramBlockName];

								paramPtr->setParamBlockBuffer(paramBlockName, blockBuffer);
							}
						}

						// Create non-shareable ones
						const GpuParamDesc& desc = paramPtr->getParamDesc();
						for (auto iterBlockDesc = desc.paramBlocks.begin(); iterBlockDesc != desc.paramBlocks.end(); ++iterBlockDesc)
						{
							if (!iterBlockDesc->second.isShareable)
							{
								ParamBlockPtrType newParamBlockBuffer = ParamBlockType::create(iterBlockDesc->second.blockSize * sizeof(UINT32));

								paramPtr->setParamBlockBuffer(iterBlockDesc->first, newParamBlockBuffer);
							}
						}
					}
				}
			}

			// Assign default parameters
			initDefaultParameters();
		}

		_markDependenciesDirty();
	}

	template <bool Core>
	template <typename T>
	void TMaterial<Core>::setParamValue(const String& name, UINT8* buffer, UINT32 numElements)
	{
		TMaterialDataParam<T, Core> param;
		getParam(name, param);

		T* ptr = (T*)buffer;
		for (UINT32 i = 0; i < numElements; i++)
			param.set(ptr[i], i);
	}

	template <bool Core>
	void TMaterial<Core>::initDefaultParameters()
	{
		const Map<String, SHADER_DATA_PARAM_DESC>& dataParams = mShader->getDataParams();
		for (auto& paramData : dataParams)
		{
			if (paramData.second.defaultValueIdx == (UINT32)-1)
				continue;

			auto iterFind = mValidParams.find(paramData.first);
			if (iterFind == mValidParams.end())
				continue;
			
			UINT8* buffer = (UINT8*)mShader->getDefaultValue(paramData.second.defaultValueIdx);
			if (buffer == nullptr)
				continue;

			switch (paramData.second.type)
			{
			case GPDT_FLOAT1: 
				setParamValue<float>(iterFind->first, buffer, paramData.second.arraySize);
				break;
			case GPDT_FLOAT2:
				setParamValue<Vector2>(iterFind->first, buffer, paramData.second.arraySize);
				break;
			case GPDT_FLOAT3:
				setParamValue<Vector3>(iterFind->first, buffer, paramData.second.arraySize);
				break;
			case GPDT_FLOAT4:
				setParamValue<Vector4>(iterFind->first, buffer, paramData.second.arraySize);
				break;
			case GPDT_MATRIX_2X2:
				setParamValue<Matrix2>(iterFind->first, buffer, paramData.second.arraySize);
				break;
			case GPDT_MATRIX_2X3:
				setParamValue<Matrix2x3>(iterFind->first, buffer, paramData.second.arraySize);
				break;
			case GPDT_MATRIX_2X4:
				setParamValue<Matrix2x4>(iterFind->first, buffer, paramData.second.arraySize);
				break;
			case GPDT_MATRIX_3X2:
				setParamValue<Matrix3x2>(iterFind->first, buffer, paramData.second.arraySize);
				break;
			case GPDT_MATRIX_3X3:
				setParamValue<Matrix3>(iterFind->first, buffer, paramData.second.arraySize);
				break;
			case GPDT_MATRIX_3X4:
				setParamValue<Matrix3x4>(iterFind->first, buffer, paramData.second.arraySize);
				break;
			case GPDT_MATRIX_4X2:
				setParamValue<Matrix4x2>(iterFind->first, buffer, paramData.second.arraySize);
				break;
			case GPDT_MATRIX_4X3:
				setParamValue<Matrix4x3>(iterFind->first, buffer, paramData.second.arraySize);
				break;
			case GPDT_MATRIX_4X4:
				setParamValue<Matrix4>(iterFind->first, buffer, paramData.second.arraySize);
				break;
			case GPDT_INT1:
				setParamValue<int>(iterFind->first, buffer, paramData.second.arraySize);
				break;
			case GPDT_INT2:
				setParamValue<Vector2I>(iterFind->first, buffer, paramData.second.arraySize);
				break;
			case GPDT_INT3:
				setParamValue<Vector3I>(iterFind->first, buffer, paramData.second.arraySize);
				break;
			case GPDT_INT4:
				setParamValue<Vector4I>(iterFind->first, buffer, paramData.second.arraySize);
				break;
			case GPDT_BOOL:
				setParamValue<int>(iterFind->first, buffer, paramData.second.arraySize);
				break;
			case GPDT_COLOR:
				setParamValue<Color>(iterFind->first, buffer, paramData.second.arraySize);
				break;
			case GPDT_STRUCT:
			{
				TMaterialParamStruct<Core> param = getParamStruct(paramData.first);

				UINT32 elementSizeBytes = paramData.second.elementSize * sizeof(UINT32);
				UINT8* ptr = buffer;
				for (UINT32 i = 0; i < paramData.second.arraySize; i++)
				{
					param.set(ptr, elementSizeBytes, i);
					ptr += elementSizeBytes;
				}
			}
			break;
			}
		}

		const Map<String, SHADER_OBJECT_PARAM_DESC>& textureParams = mShader->getTextureParams();
		for (auto& param : textureParams)
		{
			if (param.second.defaultValueIdx == (UINT32)-1)
				continue;

			auto iterFind = mValidParams.find(param.first);
			if (iterFind == mValidParams.end())
				continue;

			TextureType defaultTex = mShader->getDefaultTexture(param.second.defaultValueIdx);
			getParamTexture(param.first).set(defaultTex);
		}

		const Map<String, SHADER_OBJECT_PARAM_DESC>& samplerParams = mShader->getSamplerParams();
		for (auto& param : samplerParams)
		{
			if (param.second.defaultValueIdx == (UINT32)-1)
				continue;

			auto iterFind = mValidParams.find(param.first);
			if (iterFind == mValidParams.end())
				continue;

			SamplerStateType defaultSampler = mShader->getDefaultSampler(param.second.defaultValueIdx);
			getParamSamplerState(param.first).set(defaultSampler);
		}
	}

	template <bool Core>
	template <typename T>
	void TMaterial<Core>::getParam(const String& name, TMaterialDataParam<T, Core>& output) const
	{
		throwIfNotInitialized();

		auto iterFind = mValidParams.find(name);
		if (iterFind == mValidParams.end())
		{
			LOGWRN("Material doesn't have a parameter named " + name);
			return;
		}

		const String& gpuVarName = iterFind->second;
		SPtr<Vector<TGpuDataParam<T, Core>>> gpuParams = bs_shared_ptr_new<Vector<TGpuDataParam<T, Core>>>();

		for (auto iter = mParametersPerPass.begin(); iter != mParametersPerPass.end(); ++iter)
		{
			SPtr<PassParamsType> params = *iter;

			for (UINT32 i = 0; i < PassParamsType::NUM_PARAMS; i++)
			{
				GpuParamsType& paramPtr = params->getParamByIdx(i);
				if (paramPtr)
				{
					if (paramPtr->hasParam(gpuVarName))
					{
						gpuParams->push_back(TGpuDataParam<T, Core>());
						paramPtr->getParam<T>(gpuVarName, gpuParams->back());
					}
				}
			}
		}

		output = TMaterialDataParam<T, Core>(gpuParams);
	}

	template<bool Core>
	void TMaterial<Core>::throwIfNotInitialized() const
	{
		if (mShader == nullptr)
		{
			BS_EXCEPT(InternalErrorException, "Material does not have shader set.");
		}

		if (mBestTechnique == nullptr)
		{
			BS_EXCEPT(InternalErrorException, "Shader does not contain a supported technique.");
		}
	}

	template class TMaterial < false > ;
	template class TMaterial < true > ;

	template BS_CORE_EXPORT void TMaterial<false>::getParam(const String&, TMaterialDataParam<float, false>&) const;
	template BS_CORE_EXPORT void TMaterial<false>::getParam(const String&, TMaterialDataParam<int, false>&) const;
	template BS_CORE_EXPORT void TMaterial<false>::getParam(const String&, TMaterialDataParam<Color, false>&) const;
	template BS_CORE_EXPORT void TMaterial<false>::getParam(const String&, TMaterialDataParam<Vector2, false>&) const;
	template BS_CORE_EXPORT void TMaterial<false>::getParam(const String&, TMaterialDataParam<Vector3, false>&) const;
	template BS_CORE_EXPORT void TMaterial<false>::getParam(const String&, TMaterialDataParam<Vector4, false>&) const;
	template BS_CORE_EXPORT void TMaterial<false>::getParam(const String&, TMaterialDataParam<Vector2I, false>&) const;
	template BS_CORE_EXPORT void TMaterial<false>::getParam(const String&, TMaterialDataParam<Vector3I, false>&) const;
	template BS_CORE_EXPORT void TMaterial<false>::getParam(const String&, TMaterialDataParam<Vector4I, false>&) const;
	template BS_CORE_EXPORT void TMaterial<false>::getParam(const String&, TMaterialDataParam<Matrix2, false>&) const;
	template BS_CORE_EXPORT void TMaterial<false>::getParam(const String&, TMaterialDataParam<Matrix2x3, false>&) const;
	template BS_CORE_EXPORT void TMaterial<false>::getParam(const String&, TMaterialDataParam<Matrix2x4, false>&) const;
	template BS_CORE_EXPORT void TMaterial<false>::getParam(const String&, TMaterialDataParam<Matrix3, false>&) const;
	template BS_CORE_EXPORT void TMaterial<false>::getParam(const String&, TMaterialDataParam<Matrix3x2, false>&) const;
	template BS_CORE_EXPORT void TMaterial<false>::getParam(const String&, TMaterialDataParam<Matrix3x4, false>&) const;
	template BS_CORE_EXPORT void TMaterial<false>::getParam(const String&, TMaterialDataParam<Matrix4, false>&) const;
	template BS_CORE_EXPORT void TMaterial<false>::getParam(const String&, TMaterialDataParam<Matrix4x2, false>&) const;
	template BS_CORE_EXPORT void TMaterial<false>::getParam(const String&, TMaterialDataParam<Matrix4x3, false>&) const;

	template BS_CORE_EXPORT void TMaterial<true>::getParam(const String&, TMaterialDataParam<float, true>&) const;
	template BS_CORE_EXPORT void TMaterial<true>::getParam(const String&, TMaterialDataParam<int, true>&) const;
	template BS_CORE_EXPORT void TMaterial<true>::getParam(const String&, TMaterialDataParam<Color, true>&) const;
	template BS_CORE_EXPORT void TMaterial<true>::getParam(const String&, TMaterialDataParam<Vector2, true>&) const;
	template BS_CORE_EXPORT void TMaterial<true>::getParam(const String&, TMaterialDataParam<Vector3, true>&) const;
	template BS_CORE_EXPORT void TMaterial<true>::getParam(const String&, TMaterialDataParam<Vector4, true>&) const;
	template BS_CORE_EXPORT void TMaterial<true>::getParam(const String&, TMaterialDataParam<Vector2I, true>&) const;
	template BS_CORE_EXPORT void TMaterial<true>::getParam(const String&, TMaterialDataParam<Vector3I, true>&) const;
	template BS_CORE_EXPORT void TMaterial<true>::getParam(const String&, TMaterialDataParam<Vector4I, true>&) const;
	template BS_CORE_EXPORT void TMaterial<true>::getParam(const String&, TMaterialDataParam<Matrix2, true>&) const;
	template BS_CORE_EXPORT void TMaterial<true>::getParam(const String&, TMaterialDataParam<Matrix2x3, true>&) const;
	template BS_CORE_EXPORT void TMaterial<true>::getParam(const String&, TMaterialDataParam<Matrix2x4, true>&) const;
	template BS_CORE_EXPORT void TMaterial<true>::getParam(const String&, TMaterialDataParam<Matrix3, true>&) const;
	template BS_CORE_EXPORT void TMaterial<true>::getParam(const String&, TMaterialDataParam<Matrix3x2, true>&) const;
	template BS_CORE_EXPORT void TMaterial<true>::getParam(const String&, TMaterialDataParam<Matrix3x4, true>&) const;
	template BS_CORE_EXPORT void TMaterial<true>::getParam(const String&, TMaterialDataParam<Matrix4, true>&) const;
	template BS_CORE_EXPORT void TMaterial<true>::getParam(const String&, TMaterialDataParam<Matrix4x2, true>&) const;
	template BS_CORE_EXPORT void TMaterial<true>::getParam(const String&, TMaterialDataParam<Matrix4x3, true>&) const;

	MaterialCore::MaterialCore(const SPtr<ShaderCore>& shader)
	{
		setShader(shader);
	}

	MaterialCore::MaterialCore(const SPtr<ShaderCore>& shader, const SPtr<TechniqueCore>& bestTechnique,
		const Set<String>& validShareableParamBlocks, const Map<String, String>& validParams,
		const Vector<SPtr<PassParametersCore>>& passParams)
	{
		mShader = shader;
		mBestTechnique = bestTechnique;
		mValidShareableParamBlocks = validShareableParamBlocks;
		mValidParams = validParams;

		UINT32 numPassParams = (UINT32)passParams.size();
		mParametersPerPass.resize(numPassParams);
		for (UINT32 i = 0; i < numPassParams; i++)
			mParametersPerPass[i] = passParams[i];
	}

	void MaterialCore::setShader(const SPtr<ShaderCore>& shader)
	{
		mShader = shader;

		initBestTechnique();

		_markCoreDirty();
	}

	void MaterialCore::syncToCore(const CoreSyncData& data)
	{
		char* dataPtr = (char*)data.getBuffer();

		mValidShareableParamBlocks.clear();
		mValidParams.clear();
		mParametersPerPass.clear();

		UINT32 numPasses = 0;

		dataPtr = rttiReadElem(mValidShareableParamBlocks, dataPtr);
		dataPtr = rttiReadElem(mValidParams, dataPtr);
		dataPtr = rttiReadElem(numPasses, dataPtr);

		for (UINT32 i = 0; i < numPasses; i++)
		{
			SPtr<PassParametersCore>* passParameters = (SPtr<PassParametersCore>*)dataPtr;

			mParametersPerPass.push_back(*passParameters);

			passParameters->~SPtr<PassParametersCore>();
			dataPtr += sizeof(SPtr<PassParametersCore>);
		}

		SPtr<ShaderCore>* shader = (SPtr<ShaderCore>*)dataPtr;
		mShader = *shader;
		shader->~SPtr<ShaderCore>();
		dataPtr += sizeof(SPtr<ShaderCore>);

		SPtr<TechniqueCore>* technique = (SPtr<TechniqueCore>*)dataPtr;
		mBestTechnique = *technique;
		technique->~SPtr<TechniqueCore>();
		dataPtr += sizeof(SPtr<TechniqueCore>);
	}

	SPtr<MaterialCore> MaterialCore::create(const SPtr<ShaderCore>& shader)
	{
		MaterialCore* material = new (bs_alloc<MaterialCore>()) MaterialCore(shader);
		SPtr<MaterialCore> materialPtr = bs_shared_ptr<MaterialCore>(material);
		materialPtr->_setThisPtr(materialPtr);
		materialPtr->initialize();

		return materialPtr;
	}

	Material::Material()
		:mLoadFlags(Load_None)
	{ }

	Material::Material(const HShader& shader)
		:mLoadFlags(Load_None)
	{
		mShader = shader;
	}

	void Material::initialize()
	{
		setShader(mShader); // Not calling directly in constructor because it calls virtual methods

		Resource::initialize();
	}

	void Material::setShader(const HShader& shader)
	{
		mShader = shader;
		mBestTechnique = nullptr;
		mLoadFlags = Load_None;
		_markResourcesDirty();

		initializeIfLoaded();
	}

	void Material::_markCoreDirty()
	{
		markCoreDirty();
	}

	void Material::_markDependenciesDirty()
	{
		markDependenciesDirty();
	}

	void Material::_markResourcesDirty()
	{
		markListenerResourcesDirty();
	}

	SPtr<MaterialCore> Material::getCore() const
	{
		return std::static_pointer_cast<MaterialCore>(mCoreSpecific);
	}

	SPtr<CoreObjectCore> Material::createCore() const
	{
		MaterialCore* material = nullptr;

		SPtr<ShaderCore> shader;
		if (mShader != nullptr)
		{
			shader = mShader->getCore();

			if (mBestTechnique != nullptr)
			{
				SPtr<TechniqueCore> technique = mBestTechnique->getCore();

				Vector<SPtr<PassParametersCore>> passParams;
				for (auto& passParam : mParametersPerPass)
					passParams.push_back(convertParamsToCore(passParam));

				material = new (bs_alloc<MaterialCore>()) MaterialCore(shader, technique, mValidShareableParamBlocks, mValidParams, passParams);
			}
		}
		
		if (material == nullptr)
			material = new (bs_alloc<MaterialCore>()) MaterialCore(shader);

		SPtr<MaterialCore> materialPtr = bs_shared_ptr<MaterialCore>(material);
		materialPtr->_setThisPtr(materialPtr);

		return materialPtr;
	}

	CoreSyncData Material::syncToCore(FrameAlloc* allocator)
	{
		UINT32 numPasses = (UINT32)mParametersPerPass.size();

		UINT32 size = sizeof(UINT32) + numPasses * sizeof(SPtr<PassParametersCore>)
			+ sizeof(SPtr<ShaderCore>) + sizeof(SPtr<TechniqueCore>) + rttiGetElemSize(mValidShareableParamBlocks)
			+ rttiGetElemSize(mValidParams);

		UINT8* buffer = allocator->alloc(size);
		char* dataPtr = (char*)buffer;
		dataPtr = rttiWriteElem(mValidShareableParamBlocks, dataPtr);
		dataPtr = rttiWriteElem(mValidParams, dataPtr);
		dataPtr = rttiWriteElem(numPasses, dataPtr);

		for (UINT32 i = 0; i < numPasses; i++)
		{
			SPtr<PassParametersCore>* passParameters = new (dataPtr) SPtr<PassParametersCore>();
			*passParameters = convertParamsToCore(mParametersPerPass[i]);

			dataPtr += sizeof(SPtr<PassParametersCore>);
		}

		SPtr<ShaderCore>* shader = new (dataPtr)SPtr<ShaderCore>();
		if (mShader != nullptr)
			*shader = mShader->getCore();
		else
			*shader = nullptr;

		dataPtr += sizeof(SPtr<ShaderCore>);

		SPtr<TechniqueCore>* technique = new (dataPtr)SPtr<TechniqueCore>();
		if (mBestTechnique != nullptr)
			*technique = mBestTechnique->getCore();
		else
			*technique = nullptr;

		dataPtr += sizeof(SPtr<TechniqueCore>);

		return CoreSyncData(buffer, size);
	}

	void Material::getCoreDependencies(Vector<CoreObject*>& dependencies)
	{
		if (mShader.isLoaded())
			dependencies.push_back(mShader.get());

		for (auto& params : mParametersPerPass)
		{
			for (UINT32 i = 0; i < PassParameters::NUM_PARAMS; i++)
			{
				GpuParamsPtr gpuParams = params->getParamByIdx(i);
				if (gpuParams != nullptr)
					dependencies.push_back(gpuParams.get());
			}
		}
	}

	void Material::getListenerResources(Vector<HResource>& resources)
	{
		bs_frame_mark();

		{
			FrameVector<HResource> temp;
			getResourceDependencies(temp);

			for (auto& entry : temp)
				resources.push_back(entry);
		}

		bs_frame_clear();
	}

	void Material::getResourceDependencies(FrameVector<HResource>& dependencies) const
	{
		if (mShader != nullptr)
			dependencies.push_back(mShader);

		if (mShader.isLoaded())
		{
			const Map<String, SHADER_OBJECT_PARAM_DESC>& textureParams = mShader->getTextureParams();

			for (auto& param : textureParams)
			{
				if (param.second.defaultValueIdx != (UINT32)-1)
				{
					HTexture defaultTex = mShader->getDefaultTexture(param.second.defaultValueIdx);
					if (defaultTex != nullptr)
						dependencies.push_back(defaultTex);
				}
			}
		}
	}

	void Material::initializeIfLoaded()
	{
		if (areDependenciesLoaded())
		{
			if (mLoadFlags != Load_All)
			{
				mLoadFlags = Load_All;

				initBestTechnique();
				markCoreDirty();
			}
		}
		else
		{
			if (mShader.isLoaded() && mLoadFlags == Load_None)
			{
				mLoadFlags = Load_Shader;
				markListenerResourcesDirty(); // Need to register resources dependent on shader now
			}
		}
	}

	void Material::notifyResourceLoaded(const HResource& resource)
	{
		initializeIfLoaded();
	}

	void Material::notifyResourceDestroyed(const HResource& resource)
	{
		initializeIfLoaded();
	}

	void Material::notifyResourceChanged(const HResource& resource)
	{
		initializeIfLoaded();
	}

	HMaterial Material::clone()
	{
		UINT32 bufferSize = 0;

		MemorySerializer serializer;
		UINT8* buffer = serializer.encode(this, bufferSize, (void*(*)(UINT32))&bs_alloc);

		std::shared_ptr<Material> cloneObj = std::static_pointer_cast<Material>(serializer.decode(buffer, bufferSize));
		bs_free(buffer);

		return static_resource_cast<Material>(gResources()._createResourceHandle(cloneObj));
	}

	HMaterial Material::create()
	{
		MaterialPtr materialPtr = MaterialManager::instance().create();

		return static_resource_cast<Material>(gResources()._createResourceHandle(materialPtr));
	}

	HMaterial Material::create(const HShader& shader)
	{
		MaterialPtr materialPtr = MaterialManager::instance().create(shader);

		return static_resource_cast<Material>(gResources()._createResourceHandle(materialPtr));
	}

	RTTITypeBase* Material::getRTTIStatic()
	{
		return MaterialRTTI::instance();
	}

	RTTITypeBase* Material::getRTTI() const
	{
		return Material::getRTTIStatic();
	}
}