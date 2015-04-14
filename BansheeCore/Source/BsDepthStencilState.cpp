#include "BsDepthStencilState.h"
#include "BsRenderStateManager.h"
#include "BsRenderAPI.h"
#include "BsDepthStencilStateRTTI.h"
#include "BsException.h"
#include "BsResources.h"

namespace BansheeEngine
{
	DepthStencilProperties::DepthStencilProperties(const DEPTH_STENCIL_STATE_DESC& desc)
		:mData(desc)
	{

	}

	DepthStencilStateCore::DepthStencilStateCore(const DEPTH_STENCIL_STATE_DESC& desc)
		: mProperties(desc)
	{

	}

	const DepthStencilProperties& DepthStencilStateCore::getProperties() const
	{
		return mProperties;
	}

	const SPtr<DepthStencilStateCore>& DepthStencilStateCore::getDefault()
	{
		return RenderStateCoreManager::instance().getDefaultDepthStencilState();
	}

	DepthStencilState::DepthStencilState(const DEPTH_STENCIL_STATE_DESC& desc)
		:mProperties(desc)
	{

	}

	SPtr<DepthStencilStateCore> DepthStencilState::getCore() const
	{
		return std::static_pointer_cast<DepthStencilStateCore>(mCoreSpecific);
	}

	SPtr<CoreObjectCore> DepthStencilState::createCore() const
	{
		return RenderStateCoreManager::instance().createDepthStencilStateInternal(mProperties.mData);
	}

	const DepthStencilStatePtr& DepthStencilState::getDefault()
	{
		return RenderStateManager::instance().getDefaultDepthStencilState();
	}

	const DepthStencilProperties& DepthStencilState::getProperties() const
	{
		return mProperties;
	}

	DepthStencilStatePtr DepthStencilState::create(const DEPTH_STENCIL_STATE_DESC& desc)
	{
		return RenderStateManager::instance().createDepthStencilState(desc);
	}

	/************************************************************************/
	/* 								RTTI		                     		*/
	/************************************************************************/

	RTTITypeBase* DepthStencilState::getRTTIStatic()
	{
		return DepthStencilStateRTTI::instance();
	}

	RTTITypeBase* DepthStencilState::getRTTI() const
	{
		return DepthStencilState::getRTTIStatic();
	}
}