//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsRenderBeastPrerequisites.h"

namespace BansheeEngine
{
	/** @addtogroup RenderBeast
	 *  @{
	 */

	/**	Contains data about an overridden sampler states for a single pass stage. */
	struct StageSamplerOverrides
	{
		SPtr<SamplerStateCore>* stateOverrides;
		UINT32 numStates;
	};

	/**	Contains data about an overridden sampler states for a single pass. */
	struct PassSamplerOverrides
	{
		StageSamplerOverrides* stages;
		UINT32 numStages;
	};

	/**	Contains data about an overridden sampler states in the entire material. */
	struct MaterialSamplerOverrides
	{
		PassSamplerOverrides* passes;
		UINT32 numPasses;
		UINT32 refCount;
	};

	/**	Helper class for generating sampler overrides. */
	class BS_BSRND_EXPORT SamplerOverrideUtility
	{
	public:
		/**
		 * Generates a set of sampler overrides for the specified material. Overrides are generates according to the
		 * provided render options. 
		 */
		static MaterialSamplerOverrides* generateSamplerOverrides(const SPtr<MaterialCore>& material, 
			const SPtr<RenderBeastOptions>& options);

		/**	Destroys sampler overrides previously generated with generateSamplerOverrides(). */
		static void destroySamplerOverrides(MaterialSamplerOverrides* overrides);

		/**
		 * Checks if the provided sampler state requires an override, in case the render options have requirements not
		 * fulfilled by current sampler state (for example filtering type).
		 */
		static bool checkNeedsOverride(const SPtr<SamplerStateCore>& samplerState, 
			const SPtr<RenderBeastOptions>& options);

		/**
		 * Generates a new sampler state override using the provided state as the basis. Overridden properties are taken
		 * from the provided render options.
		 */
		static SPtr<SamplerStateCore> generateSamplerOverride(const SPtr<SamplerStateCore>& samplerState, 
			const SPtr<RenderBeastOptions>& options);
	};

	/** @} */
}