#include "BsRasterizerState.h"
#include "BsRenderStateManager.h"
#include "BsRenderSystem.h"
#include "BsRasterizerStateRTTI.h"
#include "BsResources.h"

namespace BansheeEngine
{
	void RasterizerState::initialize(const RASTERIZER_STATE_DESC& desc)
	{
		mData = desc;

		Resource::initialize();
	}

	const RasterizerStatePtr& RasterizerState::getDefault()
	{
		return RenderStateManager::instance().getDefaultRasterizerState();
	}

	HRasterizerState RasterizerState::create(const RASTERIZER_STATE_DESC& desc)
	{
		RasterizerStatePtr rasterizerPtr = RenderStateManager::instance().createRasterizerState(desc);

		return static_resource_cast<RasterizerState>(gResources()._createResourceHandle(rasterizerPtr));
	}

	/************************************************************************/
	/* 								RTTI		                     		*/
	/************************************************************************/

	RTTITypeBase* RasterizerState::getRTTIStatic()
	{
		return RasterizerStateRTTI::instance();
	}

	RTTITypeBase* RasterizerState::getRTTI() const
	{
		return RasterizerState::getRTTIStatic();
	}
}