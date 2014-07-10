//__________________________ Banshee Project - A modern game development toolkit _________________________________//
//_____________________________________ www.banshee-project.com __________________________________________________//
//________________________ Copyright (c) 2014 Marko Pintera. All rights reserved. ________________________________//
#include "BsBlendState.h"
#include "BsRenderStateManager.h"
#include "BsRenderSystem.h"
#include "BsBlendStateRTTI.h"
#include "BsResources.h"

namespace BansheeEngine
{
	void BlendState::initialize(const BLEND_STATE_DESC& desc)
	{
		mData = desc;

		Resource::initialize();
	}

	const BlendStatePtr& BlendState::getDefault()
	{
		return RenderStateManager::instance().getDefaultBlendState();
	}

	bool BlendState::getBlendEnabled(UINT32 renderTargetIdx) const
	{
		assert(renderTargetIdx >= 0 && renderTargetIdx < BS_MAX_MULTIPLE_RENDER_TARGETS);

		return mData.renderTargetDesc[renderTargetIdx].blendEnable;
	}

	BlendFactor BlendState::getSrcBlend(UINT32 renderTargetIdx) const
	{
		assert(renderTargetIdx >= 0 && renderTargetIdx < BS_MAX_MULTIPLE_RENDER_TARGETS);

		return mData.renderTargetDesc[renderTargetIdx].srcBlend;
	}

	BlendFactor BlendState::getDstBlend(UINT32 renderTargetIdx) const
	{
		assert(renderTargetIdx >= 0 && renderTargetIdx < BS_MAX_MULTIPLE_RENDER_TARGETS);

		return mData.renderTargetDesc[renderTargetIdx].dstBlend;
	}

	BlendOperation BlendState::getBlendOperation(UINT32 renderTargetIdx) const
	{
		assert(renderTargetIdx >= 0 && renderTargetIdx < BS_MAX_MULTIPLE_RENDER_TARGETS);

		return mData.renderTargetDesc[renderTargetIdx].blendOp;
	}

	BlendFactor BlendState::getAlphaSrcBlend(UINT32 renderTargetIdx) const
	{
		assert(renderTargetIdx >= 0 && renderTargetIdx < BS_MAX_MULTIPLE_RENDER_TARGETS);

		return mData.renderTargetDesc[renderTargetIdx].srcBlendAlpha;
	}

	BlendFactor BlendState::getAlphaDstBlend(UINT32 renderTargetIdx) const
	{
		assert(renderTargetIdx >= 0 && renderTargetIdx < BS_MAX_MULTIPLE_RENDER_TARGETS);
		
		return mData.renderTargetDesc[renderTargetIdx].dstBlendAlpha;
	}

	BlendOperation BlendState::getAlphaBlendOperation(UINT32 renderTargetIdx) const
	{
		assert(renderTargetIdx >= 0 && renderTargetIdx < BS_MAX_MULTIPLE_RENDER_TARGETS);

		return mData.renderTargetDesc[renderTargetIdx].blendOpAlpha;
	}

	UINT8 BlendState::getRenderTargetWriteMask(UINT32 renderTargetIdx) const
	{
		assert(renderTargetIdx >= 0 && renderTargetIdx < BS_MAX_MULTIPLE_RENDER_TARGETS);

		return mData.renderTargetDesc[renderTargetIdx].renderTargetWriteMask;
	}

	HBlendState BlendState::create(const BLEND_STATE_DESC& desc)
	{
		BlendStatePtr blendStatePtr = RenderStateManager::instance().createBlendState(desc);

		return static_resource_cast<BlendState>(gResources()._createResourceHandle(blendStatePtr));
	}

	/************************************************************************/
	/* 								RTTI		                     		*/
	/************************************************************************/

	RTTITypeBase* BlendState::getRTTIStatic()
	{
		return BlendStateRTTI::instance();
	}

	RTTITypeBase* BlendState::getRTTI() const
	{
		return BlendState::getRTTIStatic();
	}
}