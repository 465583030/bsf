//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVulkanGLSLProgramFactory.h"
#include "BsVulkanGpuProgram.h"

#define AMD_EXTENSIONS
#include "glslang/Public/ShaderLang.h"

namespace bs { namespace ct
{
    const String VulkanGLSLProgramFactory::LANGUAGE_NAME = "glsl";

	VulkanGLSLProgramFactory::VulkanGLSLProgramFactory()
	{
		glslang::InitializeProcess();
	}

	VulkanGLSLProgramFactory::~VulkanGLSLProgramFactory()
	{
		glslang::FinalizeProcess();
	}

    const String& VulkanGLSLProgramFactory::getLanguage(void) const
    {
        return LANGUAGE_NAME;
    }

	SPtr<GpuProgramCore> VulkanGLSLProgramFactory::create(const GPU_PROGRAM_DESC& desc, GpuDeviceFlags deviceMask)
    {
		SPtr<GpuProgramCore> gpuProg = bs_shared_ptr<VulkanGpuProgramCore>(new (bs_alloc<VulkanGpuProgramCore>())
			VulkanGpuProgramCore(desc, deviceMask));
		gpuProg->_setThisPtr(gpuProg);

		return gpuProg;
    }

	SPtr<GpuProgramCore> VulkanGLSLProgramFactory::create(GpuProgramType type, GpuDeviceFlags deviceMask)
	{
		GPU_PROGRAM_DESC desc;
		desc.type = type;

		SPtr<GpuProgramCore> gpuProg = bs_shared_ptr<VulkanGpuProgramCore>(new (bs_alloc<VulkanGpuProgramCore>())
			VulkanGpuProgramCore(desc, deviceMask));
		gpuProg->_setThisPtr(gpuProg);

		return gpuProg;
	}
}}