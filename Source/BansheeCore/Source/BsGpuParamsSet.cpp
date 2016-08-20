//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGpuParamsSet.h"
#include "BsShader.h"
#include "BsTechnique.h"
#include "BsPass.h"
#include "BsGpuProgram.h"
#include "BsMaterialParams.h"
#include "BsGpuParamDesc.h"
#include "BsRenderAPI.h"
#include "BsGpuParamBlockBuffer.h"

namespace BansheeEngine
{
	struct ShaderBlockDesc
	{
		String name;
		GpuParamBlockUsage usage;
		int size;
		bool external;
		UINT32 sequentialIdx;
	};

	Vector<SPtr<GpuParamDesc>> getAllParamDescs(const SPtr<Technique>& technique)
	{
		Vector<SPtr<GpuParamDesc>> allParamDescs;

		// Make sure all gpu programs are fully loaded
		for (UINT32 i = 0; i < technique->getNumPasses(); i++)
		{
			SPtr<Pass> curPass = technique->getPass(i);

			SPtr<GpuProgram> vertProgram = curPass->getVertexProgram();
			if (vertProgram)
			{
				vertProgram->blockUntilCoreInitialized();
				allParamDescs.push_back(vertProgram->getParamDesc());
			}

			SPtr<GpuProgram> fragProgram = curPass->getFragmentProgram();
			if (fragProgram)
			{
				fragProgram->blockUntilCoreInitialized();
				allParamDescs.push_back(fragProgram->getParamDesc());
			}

			SPtr<GpuProgram> geomProgram = curPass->getGeometryProgram();
			if (geomProgram)
			{
				geomProgram->blockUntilCoreInitialized();
				allParamDescs.push_back(geomProgram->getParamDesc());
			}

			SPtr<GpuProgram> hullProgram = curPass->getHullProgram();
			if (hullProgram)
			{
				hullProgram->blockUntilCoreInitialized();
				allParamDescs.push_back(hullProgram->getParamDesc());
			}

			SPtr<GpuProgram> domainProgram = curPass->getDomainProgram();
			if (domainProgram)
			{
				domainProgram->blockUntilCoreInitialized();
				allParamDescs.push_back(domainProgram->getParamDesc());
			}

			SPtr<GpuProgram> computeProgram = curPass->getComputeProgram();
			if (computeProgram)
			{
				computeProgram->blockUntilCoreInitialized();
				allParamDescs.push_back(computeProgram->getParamDesc());
			}
		}

		return allParamDescs;
	}

	Vector<SPtr<GpuParamDesc>> getAllParamDescs(const SPtr<TechniqueCore>& technique)
	{
		Vector<SPtr<GpuParamDesc>> allParamDescs;

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

	bool areParamsEqual(const GpuParamDataDesc& paramA, const GpuParamDataDesc& paramB, bool ignoreBufferOffsets)
	{
		bool equal = paramA.arraySize == paramB.arraySize && paramA.elementSize == paramB.elementSize
			&& paramA.type == paramB.type && paramA.arrayElementStride == paramB.arrayElementStride;

		if (!ignoreBufferOffsets)
			equal &= paramA.cpuMemOffset == paramB.cpuMemOffset && paramA.gpuMemOffset == paramB.gpuMemOffset;

		return equal;
	}

	Vector<ShaderBlockDesc> determineValidShareableParamBlocks(const Vector<SPtr<GpuParamDesc>>& paramDescs,
		const Map<String, SHADER_PARAM_BLOCK_DESC>& shaderDesc)
	{
		struct BlockInfo
		{
			BlockInfo() { }
			BlockInfo(const String& name, const SPtr<GpuParamDesc>& paramDesc, bool isValid = true)
				:name(name), paramDesc(paramDesc), isValid(isValid)
			{ }

			String name;
			SPtr<GpuParamDesc> paramDesc;
			bool isValid;
		};

		// Make sure param blocks with the same name actually contain the same fields
		Map<String, BlockInfo> uniqueParamBlocks;

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
					uniqueParamBlocks[blockIter->first] = BlockInfo(blockIter->first, *iter);
					continue;
				}

				// The block was already determined as invalid, no need to check further
				if (!iterFind->second.isValid)
					continue;

				String otherBlockName = iterFind->second.name;
				SPtr<GpuParamDesc> otherDesc = iterFind->second.paramDesc;

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
					LOGWRN("Found two param blocks with the same name but different contents: " + blockIter->first);
					uniqueParamBlocks[blockIter->first] = BlockInfo(blockIter->first, nullptr, false);
				}
			}
		}

		Vector<ShaderBlockDesc> output;
		for (auto& entry : uniqueParamBlocks)
		{
			if (!entry.second.isValid)
				continue;

			ShaderBlockDesc shaderBlockDesc;
			shaderBlockDesc.external = false;
			shaderBlockDesc.usage = GPBU_STATIC;
			shaderBlockDesc.size = 0;
			shaderBlockDesc.name = entry.first;

			auto iterFind = shaderDesc.find(entry.first);
			if (iterFind != shaderDesc.end())
			{
				shaderBlockDesc.external = iterFind->second.shared || iterFind->second.rendererSemantic != StringID::NONE;
				shaderBlockDesc.usage = iterFind->second.usage;
			}

			for (auto iter2 = paramDescs.begin(); iter2 != paramDescs.end(); ++iter2)
			{
				auto findParamBlockDesc = (*iter2)->paramBlocks.find(entry.first);

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

	Map<String, const GpuParamDataDesc*> determineValidDataParameters(const Vector<SPtr<GpuParamDesc>>& paramDescs)
	{
		Map<String, const GpuParamDataDesc*> foundDataParams;
		Map<String, bool> validParams;

		for (auto iter = paramDescs.begin(); iter != paramDescs.end(); ++iter)
		{
			const GpuParamDesc& curDesc = **iter;

			// Check regular data params
			for (auto iter2 = curDesc.params.begin(); iter2 != curDesc.params.end(); ++iter2)
			{
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

	Vector<const GpuParamObjectDesc*> determineValidObjectParameters(const Vector<SPtr<GpuParamDesc>>& paramDescs)
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

			// Check load-store texture params
			for (auto iter2 = curDesc.loadStoreTextures.begin(); iter2 != curDesc.loadStoreTextures.end(); ++iter2)
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

	Map<String, String> determineParameterToBlockMapping(const Vector<SPtr<GpuParamDesc>>& paramDescs)
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

	UnorderedSet<String> determineValidParameters(const Vector<SPtr<GpuParamDesc>>& paramDescs,
		const Map<String, SHADER_DATA_PARAM_DESC>& dataParams,
		const Map<String, SHADER_OBJECT_PARAM_DESC>& textureParams,
		const Map<String, SHADER_OBJECT_PARAM_DESC>& bufferParams,
		const Map<String, SHADER_OBJECT_PARAM_DESC>& samplerParams)
	{
		UnorderedSet<String> validParams;

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

			validParams.insert(iter->first);
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
							validParams.insert(iter->first);
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

	template<bool Core>
	const UINT32 TGpuParamsSet<Core>::NUM_STAGES = 6;

	template<bool Core>
	TGpuParamsSet<Core>::TGpuParamsSet(const SPtr<TechniqueType>& technique, const ShaderType& shader, UINT32 techniqueIdx,
		const SPtr<MaterialParamsType>& params)
		:mPassParams(technique->getNumPasses()), mTechniqueIdx(techniqueIdx)
	{
		UINT32 numPasses = technique->getNumPasses();

		// Create GpuParams for each pass and shader stage
		for (UINT32 i = 0; i < numPasses; i++)
		{
			SPtr<PassType> curPass = technique->getPass(i);

			GpuProgramPtrType vertProgram = curPass->getVertexProgram();
			if (vertProgram)
				mPassParams[i].vertex = vertProgram->createParameters();

			GpuProgramPtrType fragProgram = curPass->getFragmentProgram();
			if (fragProgram)
				mPassParams[i].fragment = fragProgram->createParameters();

			GpuProgramPtrType geomProgram = curPass->getGeometryProgram();
			if (geomProgram)
				mPassParams[i].geometry = geomProgram->createParameters();

			GpuProgramPtrType hullProgram = curPass->getHullProgram();
			if (hullProgram)
				mPassParams[i].hull = hullProgram->createParameters();

			GpuProgramPtrType domainProgram = curPass->getDomainProgram();
			if (domainProgram)
				mPassParams[i].domain = domainProgram->createParameters();

			GpuProgramPtrType computeProgram = curPass->getComputeProgram();
			if (computeProgram)
				mPassParams[i].compute = computeProgram->createParameters();
		}

		// Create and assign parameter block buffers
		Vector<SPtr<GpuParamDesc>> allParamDescs = getAllParamDescs(technique);

		//// Fill out various helper structures
		Vector<ShaderBlockDesc> paramBlockData = determineValidShareableParamBlocks(allParamDescs, shader->getParamBlocks());
		UnorderedSet<String> validParams = determineValidParameters(
			allParamDescs, 
			shader->getDataParams(), 
			shader->getTextureParams(), 
			shader->getBufferParams(), 
			shader->getSamplerParams());

		Map<String, ParamBlockPtrType> paramBlockBuffers;

		//// Create param blocks
		for (auto& paramBlock : paramBlockData)
		{
			ParamBlockPtrType newParamBlockBuffer;
			if (!paramBlock.external)
				newParamBlockBuffer = ParamBlockType::create(paramBlock.size, paramBlock.usage);

			paramBlock.sequentialIdx = (UINT32)mBlocks.size();

			paramBlockBuffers[paramBlock.name] = newParamBlockBuffer;
			mBlocks.push_back(BlockInfo(paramBlock.name, newParamBlockBuffer, true));
		}

		//// Assign param block buffers and generate information about data parameters
		for (UINT32 i = 0; i < numPasses; i++)
		{
			for (UINT32 j = 0; j < NUM_STAGES; j++)
			{
				GpuParamsType paramPtr = getParamByIdx(j, i);
				if (paramPtr != nullptr)
				{
					// Assign shareable buffers
					UINT32 paramBlockIdx = 0;
					for (auto& block : paramBlockData)
					{
						const String& paramBlockName = block.name;
						if (paramPtr->hasParamBlock(paramBlockName))
						{
							ParamBlockPtrType blockBuffer = paramBlockBuffers[paramBlockName];

							paramPtr->setParamBlockBuffer(paramBlockName, blockBuffer);
						}

						paramBlockIdx++;
					}

					// Create non-shareable ones (these are buffers defined by default by the RHI usually)
					const GpuParamDesc& desc = paramPtr->getParamDesc();
					for (auto iterBlockDesc = desc.paramBlocks.begin(); iterBlockDesc != desc.paramBlocks.end(); ++iterBlockDesc)
					{
						const GpuParamBlockDesc& blockDesc = iterBlockDesc->second;

						UINT32 globalBlockIdx = (UINT32)-1;
						if (!blockDesc.isShareable)
						{
							ParamBlockPtrType newParamBlockBuffer = ParamBlockType::create(blockDesc.blockSize * sizeof(UINT32));

							globalBlockIdx = (UINT32)mBlocks.size();

							paramPtr->setParamBlockBuffer(iterBlockDesc->first, newParamBlockBuffer);
							mBlocks.push_back(BlockInfo(iterBlockDesc->first, newParamBlockBuffer, false));
						}
						else
						{
							auto iterFind = std::find_if(paramBlockData.begin(), paramBlockData.end(), [&](const auto& x)
							{
								return x.name == iterBlockDesc->first;
							});

							if(iterFind != paramBlockData.end())
								globalBlockIdx = iterFind->sequentialIdx;
						}

						// If this parameter block is valid, create data/struct mappings for it
						if (globalBlockIdx == (UINT32)-1)
							continue;

						for(auto& dataParam : desc.params)
						{
							if (dataParam.second.paramBlockSlot != blockDesc.slot)
								continue;

							if (validParams.count(dataParam.first) == 0)
								continue;

							UINT32 paramIdx = params->getParamIndex(dataParam.first);

							// Parameter shouldn't be in the valid parameter list if it cannot be found
							assert(paramIdx != (UINT32)-1);

							mDataParamInfos.push_back(DataParamInfo());
							DataParamInfo& paramInfo = mDataParamInfos.back();
							paramInfo.paramIdx = paramIdx;
							paramInfo.blockIdx = globalBlockIdx;
							paramInfo.offset = dataParam.second.cpuMemOffset;
						}
					}
				}
			}
		}

		// Generate information about object parameters
		bs_frame_mark();
		{
			FrameVector<ObjectParamInfo> objParamInfos;

			UINT32 offsetsSize = numPasses * NUM_STAGES * 4 * sizeof(UINT32);
			UINT32* offsets = (UINT32*)bs_frame_alloc(offsetsSize);
			memset(offsets, 0, offsetsSize);

			// First store all objects in temporary arrays since we don't know how many of them are
			UINT32 totalNumObjects = 0;
			UINT32* stageOffsets = offsets;
			for (UINT32 i = 0; i < numPasses; i++)
			{
				for (UINT32 j = 0; j < NUM_STAGES; j++)
				{
					GpuParamsType paramPtr = getParamByIdx(j, i);
					if (paramPtr == nullptr)
					{
						stageOffsets += sizeof(UINT32) * 4;
						continue;
					}

					auto processObjectParams = [&](const Map<String, GpuParamObjectDesc>& gpuParams, 
						UINT32 stageIdx, MaterialParams::ParamType paramType)
					{
						for (auto& param : gpuParams)
						{
							if (validParams.count(param.first) == 0)
								continue;

							UINT32 paramIdx = params->getParamIndex(param.first);

							// Parameter shouldn't be in the valid parameter list if it cannot be found
							assert(paramIdx != (UINT32)-1);

							objParamInfos.push_back(ObjectParamInfo());
							ObjectParamInfo& paramInfo = objParamInfos.back();
							paramInfo.paramIdx = paramIdx;
							paramInfo.slotIdx = param.second.slot;

							offsets[stageIdx]++;
							totalNumObjects++;
						}
					};

					const GpuParamDesc& desc = paramPtr->getParamDesc();
					processObjectParams(desc.textures, 0, MaterialParams::ParamType::Texture);
					processObjectParams(desc.loadStoreTextures, 1, MaterialParams::ParamType::Texture);
					processObjectParams(desc.buffers, 2, MaterialParams::ParamType::Buffer);
					processObjectParams(desc.samplers, 3, MaterialParams::ParamType::Sampler);

					stageOffsets += sizeof(UINT32) * 4;
				}
			}

			// Transfer all objects into their permanent storage
			UINT32 objectParamInfosSize = totalNumObjects * sizeof(ObjectParamInfo) + numPasses * sizeof(PassParamInfo);
			mPassParamInfos = (PassParamInfo*)bs_alloc(objectParamInfosSize);
			memset(mPassParamInfos, 0, objectParamInfosSize);

			StageParamInfo* stageInfos = (StageParamInfo*)mPassParamInfos;

			ObjectParamInfo* objInfos = (ObjectParamInfo*)(mPassParamInfos + numPasses);
			memcpy(objInfos, objParamInfos.data(), totalNumObjects * sizeof(ObjectParamInfo));

			UINT32 objInfoOffset = 0;

			stageOffsets = offsets;
			for (UINT32 i = 0; i < numPasses; i++)
			{
				for (UINT32 j = 0; j < NUM_STAGES; j++)
				{
					StageParamInfo& stage = stageInfos[i * NUM_STAGES + j];

					if(stageOffsets[0] > 0)
					{
						UINT32 numEntries = stageOffsets[0];

						stage.textures = objInfos + objInfoOffset;
						stage.numTextures = numEntries;

						objInfoOffset += numEntries;
					}

					if (stageOffsets[1] > 0)
					{
						UINT32 numEntries = stageOffsets[1];

						stage.loadStoreTextures = objInfos + objInfoOffset;
						stage.numLoadStoreTextures = numEntries;

						objInfoOffset += numEntries;
					}

					if (stageOffsets[2] > 0)
					{
						UINT32 numEntries = stageOffsets[2];

						stage.buffers = objInfos + objInfoOffset;
						stage.numBuffers = numEntries;

						objInfoOffset += numEntries;
					}

					if (stageOffsets[3] > 0)
					{
						UINT32 numEntries = stageOffsets[3];

						stage.samplerStates = objInfos + objInfoOffset;
						stage.numSamplerStates = numEntries;

						objInfoOffset += numEntries;
					}

					stageOffsets += sizeof(UINT32) * 4;
				}
			}

			bs_frame_free(offsets);
		}
		bs_frame_clear();
	}

	template<bool Core>
	TGpuParamsSet<Core>::~TGpuParamsSet()
	{
		// All allocations share the same memory, so we just clear it all at once
		bs_free(mPassParamInfos);
	}

	template<bool Core>
	typename TGpuParamsSet<Core>::GpuParamsType TGpuParamsSet<Core>::getGpuParams(GpuProgramType type, UINT32 passIdx)
	{
		if (passIdx >= mPassParams.size())
			return nullptr;

		switch (type)
		{
		case GPT_VERTEX_PROGRAM:
			return mPassParams[passIdx].vertex;
		case GPT_FRAGMENT_PROGRAM:
			return mPassParams[passIdx].fragment;
		case GPT_GEOMETRY_PROGRAM:
			return mPassParams[passIdx].geometry;
		case GPT_HULL_PROGRAM:
			return mPassParams[passIdx].hull;
		case GPT_DOMAIN_PROGRAM:
			return mPassParams[passIdx].domain;
		case GPT_COMPUTE_PROGRAM:
			return mPassParams[passIdx].compute;
		}

		return nullptr;
	}

	template<bool Core>
	void TGpuParamsSet<Core>::setParamBlockBuffer(const String& name, const ParamBlockPtrType& paramBlock)
	{
		UINT32 foundIdx = (UINT32)-1;
		for(UINT32 i = 0; i < (UINT32)mBlocks.size(); i++)
		{
			BlockInfo& block = mBlocks[i];
			if(block.name == name)
			{
				if (!block.shareable)
				{
					LOGERR("Cannot set parameter block buffer with the name \"" + name + "\". Buffer is not assignable. ");
					return;
				}

				foundIdx = i;
			}
		}

		if(foundIdx == (UINT32)-1)
		{
			LOGERR("Cannot set parameter block buffer with the name \"" + name + "\". Buffer name not found. ");
			return;
		}

		mBlocks[foundIdx].buffer = paramBlock;

		UINT32 numPasses = (UINT32)mPassParams.size();
		for (UINT32 j = 0; j < numPasses; j++)
		{
			for (UINT32 i = 0; i < NUM_STAGES; i++)
			{
				GpuParamsType paramPtr = getParamByIdx(i);
				if (paramPtr != nullptr)
				{
					if (paramPtr->hasParamBlock(name))
						paramPtr->setParamBlockBuffer(name, paramBlock);
				}
			}
		}
	}

	template<bool Core>
	void TGpuParamsSet<Core>::update(const SPtr<MaterialParamsType>& params, bool updateAll)
	{
		// Note: Instead of iterating over every single parameter, it might be more efficient for @p params to keep
		// a ring buffer and a version number. Then we could just iterate over the ring buffer and only access dirty
		// parameters. If the version number is too high (larger than ring buffer can store), then we force update for all.

		// Maximum of 31 techniques are supported. Bit 32 is reserved.
		assert(mTechniqueIdx < 31);
		UINT32 dirtyFlagCheck = 1 << mTechniqueIdx;

		// Update data params
		for(auto& paramInfo : mDataParamInfos)
		{
			ParamBlockPtrType paramBlock = mBlocks[paramInfo.blockIdx].buffer;
			if (paramBlock != nullptr)
				continue;

			const MaterialParams::ParamData* materialParamInfo = params->getParamData(paramInfo.paramIdx);
			if ((materialParamInfo->dirtyFlags & dirtyFlagCheck) == 0 && !updateAll)
				continue;

			UINT32 arraySize = materialParamInfo->arraySize == 0 ? 1 : materialParamInfo->arraySize;
			const GpuParamDataTypeInfo& typeInfo = GpuParams::PARAM_SIZES.lookup[(int)materialParamInfo->dataType];
			UINT32 paramSize = typeInfo.numColumns * typeInfo.numRows * typeInfo.baseTypeSize;

			UINT8* data = params->getData(materialParamInfo->index);

			bool transposeMatrices = RenderAPICore::instance().getAPIInfo().getGpuProgramHasColumnMajorMatrices();
			if (transposeMatrices)
			{
				auto writeTransposed = [&](auto& temp)
				{
					for (UINT32 i = 0; i < arraySize; i++)
					{
						UINT32 arrayOffset = i * paramSize;
						memcpy(&temp, data + arrayOffset, paramSize);
						temp.transpose();

						paramBlock->write(paramInfo.offset + arrayOffset, &temp, paramSize);
					}
				};

				switch (materialParamInfo->dataType)
				{
				case GPDT_MATRIX_2X2:
				{
					MatrixNxM<2, 2> matrix;
					writeTransposed(matrix);
				}
					break;
				case GPDT_MATRIX_2X3:
				{
					MatrixNxM<2, 3> matrix;
					writeTransposed(matrix);
				}
					break;
				case GPDT_MATRIX_2X4:
				{
					MatrixNxM<2, 4> matrix;
					writeTransposed(matrix);
				}
					break;
				case GPDT_MATRIX_3X2:
				{
					MatrixNxM<3, 2> matrix;
					writeTransposed(matrix);
				}
					break;
				case GPDT_MATRIX_3X3:
				{
					Matrix3 matrix;
					writeTransposed(matrix);
				}
					break;
				case GPDT_MATRIX_3X4:
				{
					MatrixNxM<3, 4> matrix;
					writeTransposed(matrix);
				}
					break;
				case GPDT_MATRIX_4X2:
				{
					MatrixNxM<4, 2> matrix;
					writeTransposed(matrix);
				}
					break;
				case GPDT_MATRIX_4X3:
				{
					MatrixNxM<4, 3> matrix;
					writeTransposed(matrix);
				}
					break;
				case GPDT_MATRIX_4X4:
				{
					Matrix4 matrix;
					writeTransposed(matrix);
				}
					break;
				default:
				{
					paramBlock->write(paramInfo.offset, data, paramSize * arraySize);
					break;
				}
				}
			}
			else
				paramBlock->write(paramInfo.offset, data, paramSize * arraySize);
		}

		// Update object params
		UINT32 numPasses = (UINT32)mPassParams.size();

		for(UINT32 i = 0; i < numPasses; i++)
		{
			for(UINT32 j = 0; j < NUM_STAGES; j++)
			{
				GpuParamsType paramsPtr = getParamByIdx(j, i);
				if(paramsPtr != nullptr)
				{
					const StageParamInfo& stageInfo = mPassParamInfos[i].stages[j];

					for(UINT32 k = 0; k < stageInfo.numTextures; k++)
					{
						const ObjectParamInfo& paramInfo = stageInfo.textures[k];

						const MaterialParams::ParamData* materialParamInfo = params->getParamData(paramInfo.paramIdx);
						if ((materialParamInfo->dirtyFlags & dirtyFlagCheck) == 0 && !updateAll)
							continue;

						TextureType texture;
						params->getTexture(materialParamInfo->index, texture);

						paramsPtr->setTexture(paramInfo.slotIdx, texture);
					}

					for (UINT32 k = 0; k < stageInfo.numLoadStoreTextures; k++)
					{
						const ObjectParamInfo& paramInfo = stageInfo.loadStoreTextures[k];

						const MaterialParams::ParamData* materialParamInfo = params->getParamData(paramInfo.paramIdx);
						if ((materialParamInfo->dirtyFlags & dirtyFlagCheck) == 0 && !updateAll)
							continue;

						TextureSurface surface;
						TextureType texture;
						params->getLoadStoreTexture(materialParamInfo->index, texture, surface);

						paramsPtr->setLoadStoreTexture(paramInfo.slotIdx, texture, surface);
					}

					for (UINT32 k = 0; k < stageInfo.numBuffers; k++)
					{
						const ObjectParamInfo& paramInfo = stageInfo.buffers[k];

						const MaterialParams::ParamData* materialParamInfo = params->getParamData(paramInfo.paramIdx);
						if ((materialParamInfo->dirtyFlags & dirtyFlagCheck) == 0 && !updateAll)
							continue;

						BufferType buffer;
						params->getBuffer(materialParamInfo->index, buffer);

						paramsPtr->setBuffer(paramInfo.slotIdx, buffer);
					}

					for (UINT32 k = 0; k < stageInfo.numSamplerStates; k++)
					{
						const ObjectParamInfo& paramInfo = stageInfo.samplerStates[k];

						const MaterialParams::ParamData* materialParamInfo = params->getParamData(paramInfo.paramIdx);
						if ((materialParamInfo->dirtyFlags & dirtyFlagCheck) == 0 && !updateAll)
							continue;

						SamplerStateType samplerState;
						params->getSamplerState(materialParamInfo->index, samplerState);

						paramsPtr->setSamplerState(paramInfo.slotIdx, samplerState);
					}

					paramsPtr->_markCoreDirty();
				}
			}
		}
	}

	template class TGpuParamsSet <false>;
	template class TGpuParamsSet <true>;
}