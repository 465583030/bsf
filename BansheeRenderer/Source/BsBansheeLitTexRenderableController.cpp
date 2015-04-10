#include "BsBansheeLitTexRenderableController.h"
#include "BsShader.h"
#include "BsGpuParams.h"
#include "BsBansheeRenderer.h"
#include "BsHardwareBufferManager.h"
#include "BsGpuParamBlockBuffer.h"
#include "BsTechnique.h"
#include "BsPass.h"
#include "BsMaterial.h"
#include "BsRenderAPI.h"

namespace BansheeEngine
{
	LitTexRenderableController::LitTexRenderableController()
	{
		defaultShader = createDefaultShader();

		SPtr<TechniqueCore> defaultTechnique = defaultShader->getBestTechnique();
		SPtr<PassCore> defaultPass = defaultTechnique->getPass(0);

		GpuParamDescPtr vertParamDesc = defaultPass->getVertexProgram()->getParamDesc();
		GpuParamDescPtr fragParamDesc = defaultPass->getFragmentProgram()->getParamDesc();

		GpuParamDescPtr staticParamsDesc = bs_shared_ptr<GpuParamDesc>();
		GpuParamDescPtr perFrameParamsDesc = bs_shared_ptr<GpuParamDesc>();
		GpuParamDescPtr perObjectParamsDesc = bs_shared_ptr<GpuParamDesc>();

		bool foundLightDir = false;
		bool foundTime = false;
		bool foundWVP = false;
		bool foundStatic = false;
		bool foundPerFrame = false;
		bool foundPerObject = false;

		const Map<String, SHADER_DATA_PARAM_DESC>& dataParams = defaultShader->getDataParams();
		for (auto& param : dataParams)
		{
			if (!foundLightDir && param.second.rendererSemantic == RPS_LightDir)
			{
				auto iterFind = fragParamDesc->params.find(param.second.gpuVariableName);
				if (iterFind == fragParamDesc->params.end())
					continue;

				lightDirParamDesc = iterFind->second;
				staticParamsDesc->params[iterFind->first] = iterFind->second;
				foundLightDir = true;
			}
			else if (!foundTime && param.second.rendererSemantic == RPS_Time)
			{
				auto iterFind = vertParamDesc->params.find(param.second.gpuVariableName);
				if (iterFind == vertParamDesc->params.end())
					continue;

				timeParamDesc = iterFind->second;
				perFrameParamsDesc->params[iterFind->first] = iterFind->second;
				foundTime = true;
			}
			else if (!foundWVP && param.second.rendererSemantic == RPS_WorldViewProjTfrm)
			{
				auto iterFind = vertParamDesc->params.find(param.second.gpuVariableName);
				if (iterFind == vertParamDesc->params.end())
					continue;

				wvpParamDesc = iterFind->second;
				perObjectParamsDesc->params[iterFind->first] = iterFind->second;
				foundWVP = true;
			}
		}

		const Map<String, SHADER_PARAM_BLOCK_DESC>& paramBlocks = defaultShader->getParamBlocks();
		for (auto& block : paramBlocks)
		{
			if (!foundStatic && block.second.rendererSemantic == RBS_Static)
			{
				auto iterFind = fragParamDesc->paramBlocks.find(block.second.name);
				if (iterFind == fragParamDesc->paramBlocks.end())
					continue;

				staticParamBlockDesc = iterFind->second;
				staticParamsDesc->paramBlocks[iterFind->first] = iterFind->second;
				foundStatic = true;
			}
			else if (!foundPerFrame && block.second.rendererSemantic == RBS_PerFrame)
			{
				auto iterFind = vertParamDesc->paramBlocks.find(block.second.name);
				if (iterFind == vertParamDesc->paramBlocks.end())
					continue;

				perFrameParamBlockDesc = iterFind->second;
				perFrameParamsDesc->paramBlocks[iterFind->first] = iterFind->second;
				foundPerFrame = true;
			}
			else if (!foundPerObject && block.second.rendererSemantic == RBS_PerObject)
			{
				auto iterFind = vertParamDesc->paramBlocks.find(block.second.name);
				if (iterFind == vertParamDesc->paramBlocks.end())
					continue;

				perObjectParamBlockDesc = iterFind->second;
				perObjectParamsDesc->paramBlocks[iterFind->first] = iterFind->second;
				foundPerObject = true;
			}
		}

		if (!foundLightDir || !foundTime || !foundWVP || !foundStatic || !foundPerFrame || !foundPerObject)
			BS_EXCEPT(InternalErrorException, "Invalid default shader.");

		// Create global GPU param buffers and get parameter handles
		staticParams = GpuParamsCore::create(staticParamsDesc, false);
		perFrameParams = GpuParamsCore::create(perFrameParamsDesc, false);

		staticParamBuffer = HardwareBufferCoreManager::instance().createGpuParamBlockBuffer(staticParamBlockDesc.blockSize * sizeof(UINT32));
		perFrameParamBuffer = HardwareBufferCoreManager::instance().createGpuParamBlockBuffer(perFrameParamBlockDesc.blockSize * sizeof(UINT32));

		staticParams->setParamBlockBuffer(staticParamBlockDesc.slot, staticParamBuffer);
		perFrameParams->setParamBlockBuffer(perFrameParamBlockDesc.slot, perFrameParamBuffer);

		staticParams->getParam(lightDirParamDesc.name, lightDirParam);
		perFrameParams->getParam(timeParamDesc.name, timeParam);

		lightDirParam.set(Vector4(0.707f, 0.707f, 0.707f, 0.0f));
	}

	void LitTexRenderableController::initializeRenderElem(RenderableElement& element)
	{
		static auto paramsMatch = [](const GpuParamDataDesc& a, const GpuParamDataDesc& b)
		{
			return a.gpuMemOffset == b.gpuMemOffset && a.elementSize == b.elementSize &&
				a.arraySize == b.arraySize && a.arrayElementStride == b.arrayElementStride;
		};

		element.rendererData = PerObjectData();
		PerObjectData* rendererData = any_cast_unsafe<PerObjectData>(&element.rendererData);

		SPtr<ShaderCore> shader = element.material->getShader();
		if (shader == nullptr)
		{
			LOGWRN("Missing shader on material.");
			return;
		}

		const Map<String, SHADER_PARAM_BLOCK_DESC>& paramBlockDescs = shader->getParamBlocks();
		const Map<String, SHADER_DATA_PARAM_DESC>& dataParamDescs = shader->getDataParams();
		String staticBlockName;
		String perFrameBlockName;
		String perObjectBlockName;

		String wvpParamName;

		for (auto& paramBlockDesc : paramBlockDescs)
		{
			if (paramBlockDesc.second.rendererSemantic == RBS_Static)
				staticBlockName = paramBlockDesc.second.name;
			else if(paramBlockDesc.second.rendererSemantic == RBS_PerFrame)
				perFrameBlockName = paramBlockDesc.second.name;
			else if (paramBlockDesc.second.rendererSemantic == RBS_PerObject)
				perObjectBlockName = paramBlockDesc.second.name;
		}

		for (auto& paramDesc : dataParamDescs)
		{
			if (paramDesc.second.rendererSemantic == RPS_WorldViewProjTfrm)
				wvpParamName = paramDesc.second.gpuVariableName;
		}

		UINT32 numPasses = element.material->getNumPasses();
		for (UINT32 i = 0; i < numPasses; i++)
		{
			SPtr<PassParametersCore> passParams = element.material->getPassParameters(i);

			for (UINT32 j = 0; j < passParams->getNumParams(); j++)
			{
				SPtr<GpuParamsCore> gpuParams = passParams->getParamByIdx(j);
				if (gpuParams == nullptr)
					continue;

				const GpuParamDesc& paramsDesc = gpuParams->getParamDesc();

				if (staticBlockName != "")
				{
					auto findIter = paramsDesc.paramBlocks.find(staticBlockName);
					if (findIter != paramsDesc.paramBlocks.end())
					{
						// TODO - We only compare block sizes but not actual contents. Should I check them too?
						//        Probably shouldn't concern myself with that here, instead check that on a higher level.
						if (findIter->second.blockSize == staticParamBlockDesc.blockSize)
						{
							UINT32 slotIdx = findIter->second.slot;
							element.rendererBuffers.push_back(RenderableElement::BufferBindInfo(i, j, slotIdx, staticParamBuffer));
						}
					}
				}

				if (perFrameBlockName != "")
				{
					auto findIter = paramsDesc.paramBlocks.find(perFrameBlockName);
					if (findIter != paramsDesc.paramBlocks.end())
					{
						if (findIter->second.blockSize == perFrameParamBlockDesc.blockSize)
						{
							UINT32 slotIdx = findIter->second.slot;
							element.rendererBuffers.push_back(RenderableElement::BufferBindInfo(i, j, slotIdx, perFrameParamBuffer));
						}
					}
				}

				if (perObjectBlockName != "")
				{
					auto findIter = paramsDesc.paramBlocks.find(perObjectBlockName);
					if (findIter != paramsDesc.paramBlocks.end())
					{
						if (findIter->second.blockSize == perObjectParamBlockDesc.blockSize)
						{
							if (rendererData->perObjectParamBuffer == nullptr)
								rendererData->perObjectParamBuffer = HardwareBufferCoreManager::instance().createGpuParamBlockBuffer(perObjectParamBlockDesc.blockSize * sizeof(UINT32));

							rendererData->perObjectBuffers.push_back(RenderableElement::BufferBindInfo(i, j, findIter->second.slot, rendererData->perObjectParamBuffer));

							if (!rendererData->hasWVPParam && wvpParamName != "")
							{
								auto findIter2 = paramsDesc.params.find(wvpParamName);
								if (findIter2 != paramsDesc.params.end())
								{
									if (paramsMatch(findIter2->second, wvpParamDesc))
									{
										gpuParams->getParam(wvpParamName, rendererData->wvpParam);
										rendererData->hasWVPParam = true;
									}
								}
							}
						}
					}
				}
			}
		}

		bindGlobalBuffers(element);
	}

	void LitTexRenderableController::bindPerObjectBuffers(const RenderableElement& element)
	{
		const PerObjectData* rendererData = any_cast_unsafe<PerObjectData>(&element.rendererData);
		for (auto& perObjectBuffer : rendererData->perObjectBuffers)
		{
			SPtr<GpuParamsCore> params = element.material->getPassParameters(perObjectBuffer.passIdx)->getParamByIdx(perObjectBuffer.paramsIdx);

			params->setParamBlockBuffer(perObjectBuffer.slotIdx, rendererData->perObjectParamBuffer);
		}
	}

	void LitTexRenderableController::updateGlobalBuffers(float time)
	{
		timeParam.set(time);

		staticParams->updateHardwareBuffers();
		perFrameParams->updateHardwareBuffers();
	}

	void LitTexRenderableController::updatePerObjectBuffers(RenderableElement& element, const Matrix4& wvpMatrix)
	{
		PerObjectData* rendererData = any_cast_unsafe<PerObjectData>(&element.rendererData);

		if (rendererData->hasWVPParam)
			rendererData->wvpParam.set(wvpMatrix);

		if (rendererData->perObjectParamBuffer != nullptr)
			rendererData->perObjectParamBuffer->flushToGPU();
	}

	SPtr<ShaderCore> LitTexRenderableController::createDefaultShader()
	{
		StringID rsName = RenderAPICore::instance().getName();

		SPtr<GpuProgramCore> vsProgram;
		SPtr<GpuProgramCore> psProgram;

		if (rsName == RenderAPIDX11)
		{
			String vsCode = R"(
			cbuffer PerFrame
			{
				float time;
			}
				
			cbuffer PerObject
			{
				float4x4 matWorldViewProj;
			}

			void vs_main(in float3 inPos : POSITION,
					     out float4 oPosition : SV_Position)
			{
				 oPosition = mul(matWorldViewProj, float4(inPos.xyz + float3(sin(time), 0, 0), 1));
			})";

			String psCode = R"(
			cbuffer Static
			{
				float4 lightDir;
			}
			
			float4 ps_main() : SV_Target
			{
				return dot(lightDir, float4(0.5f, 0.5f, 0.5f, 0.5f));
			})";	

			vsProgram = GpuProgramCore::create(vsCode, "vs_main", "hlsl", GPT_VERTEX_PROGRAM, GPP_VS_4_0);
			psProgram = GpuProgramCore::create(psCode, "ps_main", "hlsl", GPT_FRAGMENT_PROGRAM, GPP_FS_4_0);
		}
		else if (rsName == RenderAPIDX9)
		{
			String vsCode = R"(
			BS_PARAM_BLOCK PerFrame { time }
			BS_PARAM_BLOCK PerObject { matWorldViewProj }

			float time;
			float4x4 matWorldViewProj;

			void vs_main(in float3 inPos : POSITION,
						out float4 oPosition : POSITION)
			{
				oPosition = mul(matWorldViewProj, float4(inPos.xyz + float3(sin(time), 0, 0), 1));
			})";

			String psCode = R"(
			BS_PARAM_BLOCK Static { lightDir }

			float4 lightDir;

			float4 ps_main() : COLOR0
			{
				return dot(lightDir, float4(0.5f, 0.5f, 0.5f, 0.5f));
			})";

			vsProgram = GpuProgramCore::create(vsCode, "vs_main", "hlsl", GPT_VERTEX_PROGRAM, GPP_VS_2_0);
			psProgram = GpuProgramCore::create(psCode, "ps_main", "hlsl", GPT_FRAGMENT_PROGRAM, GPP_FS_2_0);
		}
		else if (rsName == RenderAPIOpenGL)
		{
			String vsCode = R"(#version 400

			uniform PerFrame
			{
				float time;
			};

			uniform PerObject
			{
				mat4 matWorldViewProj;
			};

			in vec3 bs_position;

			void main()
			{
				gl_Position = matWorldViewProj * vec4(bs_position.xyz + vec3(sin(time), 0, 0), 1);
			})";

			String psCode = R"(#version 400

			uniform Static
			{
				vec4 lightDir;
			};

			out vec4 fragColor;

			void main()
			{
				fragColor = lightDir * vec4(0.5f, 0.5f, 0.5f, 0.5f);
			})";

			vsProgram = GpuProgramCore::create(vsCode, "main", "glsl", GPT_VERTEX_PROGRAM, GPP_VS_4_0);
			psProgram = GpuProgramCore::create(psCode, "main", "glsl", GPT_FRAGMENT_PROGRAM, GPP_FS_4_0);
		}

		PASS_DESC_CORE passDesc;
		passDesc.vertexProgram = vsProgram;
		passDesc.fragmentProgram = psProgram;

		SPtr<PassCore> newPass = PassCore::create(passDesc);
		SPtr<TechniqueCore> newTechnique = TechniqueCore::create(rsName, RendererDefault, { newPass });

		SHADER_DESC shaderDesc;
		shaderDesc.setParamBlockAttribs("Static", true, GPBU_DYNAMIC, RBS_Static);
		shaderDesc.setParamBlockAttribs("PerFrame", true, GPBU_DYNAMIC, RBS_PerFrame);
		shaderDesc.setParamBlockAttribs("PerObject", true, GPBU_DYNAMIC, RBS_PerObject);

		shaderDesc.addParameter("lightDir", "lightDir", GPDT_FLOAT4, RPS_LightDir);
		shaderDesc.addParameter("time", "time", GPDT_FLOAT1, RPS_Time);
		shaderDesc.addParameter("matWorldViewProj", "matWorldViewProj", GPDT_MATRIX_4X4, RPS_WorldViewProjTfrm);

		SPtr<ShaderCore> defaultShader = ShaderCore::create("LitTexDefault", shaderDesc, { newTechnique });

		return defaultShader;
	}
}