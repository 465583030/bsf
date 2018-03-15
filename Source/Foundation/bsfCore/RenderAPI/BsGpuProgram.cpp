//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/BsGpuProgram.h"
#include "RenderAPI/BsRenderAPICapabilities.h"
#include "RenderAPI/BsRenderAPI.h"
#include "RenderAPI/BsGpuParams.h"
#include "RenderAPI/BsGpuParamDesc.h"
#include "Managers/BsGpuProgramManager.h"
#include "Private/RTTI/BsGpuProgramRTTI.h"

namespace bs
{
	GpuProgramProperties::GpuProgramProperties(const String& source, const String& entryPoint, GpuProgramType gptype)
		:mType(gptype), mEntryPoint(entryPoint), mSource(source)
	{ }
		
	GpuProgram::GpuProgram(const GPU_PROGRAM_DESC& desc)
		: mNeedsAdjacencyInfo(desc.requiresAdjacency), mLanguage(desc.language)
		, mProperties(desc.source, desc.entryPoint, desc.type)
	{

	}

	bool GpuProgram::isCompiled() const
	{
		return getCore()->isCompiled();
	}

	String GpuProgram::getCompileErrorMessage() const
	{
		return getCore()->getCompileErrorMessage();
	}

	SPtr<GpuParamDesc> GpuProgram::getParamDesc() const
	{
		return getCore()->getParamDesc();
	}

	SPtr<ct::GpuProgram> GpuProgram::getCore() const
	{
		return std::static_pointer_cast<ct::GpuProgram>(mCoreSpecific);
	}

	SPtr<ct::CoreObject> GpuProgram::createCore() const
	{
		GPU_PROGRAM_DESC desc;
		desc.source = mProperties.getSource();
		desc.entryPoint = mProperties.getEntryPoint();
		desc.language = mLanguage;
		desc.type = mProperties.getType();
		desc.requiresAdjacency = mNeedsAdjacencyInfo;

		return ct::GpuProgramManager::instance().createInternal(desc);
	}

	SPtr<GpuProgram> GpuProgram::create(const GPU_PROGRAM_DESC& desc)
	{
		return GpuProgramManager::instance().create(desc);
	}

	/************************************************************************/
	/* 								SERIALIZATION                      		*/
	/************************************************************************/
	RTTITypeBase* GpuProgram::getRTTIStatic()
	{
		return GpuProgramRTTI::instance();
	}

	RTTITypeBase* GpuProgram::getRTTI() const
	{
		return GpuProgram::getRTTIStatic();
	}

	namespace ct
	{
	GpuProgram::GpuProgram(const GPU_PROGRAM_DESC& desc, GpuDeviceFlags deviceMask)
		:mNeedsAdjacencyInfo(desc.requiresAdjacency), mIsCompiled(false), mProperties(desc.source, desc.entryPoint, 
			desc.type)
	{
		mParametersDesc = bs_shared_ptr_new<GpuParamDesc>();
	}

	bool GpuProgram::isSupported() const
	{
		return true;
	}

	SPtr<GpuProgram> GpuProgram::create(const GPU_PROGRAM_DESC& desc, GpuDeviceFlags deviceMask)
	{
		return GpuProgramManager::instance().create(desc, deviceMask);
	}
	}
}