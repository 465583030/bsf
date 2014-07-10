//__________________________ Banshee Project - A modern game development toolkit _________________________________//
//_____________________________________ www.banshee-project.com __________________________________________________//
//________________________ Copyright (c) 2014 Marko Pintera. All rights reserved. ________________________________//
#pragma once

#include "BsD3D11Prerequisites.h"
#include "BsSamplerState.h"

namespace BansheeEngine
{
	/**
	 * @brief	DirectX 11 implementation of a sampler state. Wraps a DX11
	 *			sampler state object.
	 */
	class BS_D3D11_EXPORT D3D11SamplerState : public SamplerState
	{
	public:
		~D3D11SamplerState();
		ID3D11SamplerState* getInternal() const { return mSamplerState; }

	protected:
		friend class D3D11RenderStateManager;

		D3D11SamplerState();

		/**
		 * @copydoc SamplerState::initialize_internal
		 */
		void initialize_internal();

		/**
		 * @copydoc SamplerState::destroy_internal
		 */
		void destroy_internal();

		ID3D11SamplerState* mSamplerState;
	};
}