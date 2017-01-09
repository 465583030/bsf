//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "BsRenderStateManager.h"

namespace bs { namespace ct
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	/**	Handles creation of Vulkan pipeline states. */
	class VulkanRenderStateCoreManager : public RenderStateCoreManager
	{
	protected:
		/** @copydoc RenderStateCoreManager::createSamplerStateStateInternal */
		SPtr<SamplerStateCore> createSamplerStateInternal(const SAMPLER_STATE_DESC& desc,
			GpuDeviceFlags deviceMask) const override;

		/** @copydoc RenderStateCoreManager::_createGraphicsPipelineState */
		SPtr<GraphicsPipelineStateCore> _createGraphicsPipelineState(const PIPELINE_STATE_CORE_DESC& desc,
			GpuDeviceFlags deviceMask = GDF_DEFAULT) const override;

		/** @copydoc RenderStateCoreManager::_createComputePipelineState */
		SPtr<ComputePipelineStateCore> _createComputePipelineState(const SPtr<GpuProgramCore>& program,
			GpuDeviceFlags deviceMask = GDF_DEFAULT) const override;

		/** @copydoc RenderStateCoreManager::_createPipelineParamInfo */
		SPtr<GpuPipelineParamInfoCore> _createPipelineParamInfo(const GPU_PIPELINE_PARAMS_DESC& desc,
			 GpuDeviceFlags deviceMask = GDF_DEFAULT) const override;
	};

	/** @} */
}}