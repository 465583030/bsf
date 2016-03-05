//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsD3D11Prerequisites.h"
#include "BsOcclusionQuery.h"

namespace BansheeEngine
{
	/** @addtogroup D3D11
	 *  @{
	 */

	/** @copydoc OcclusionQuery */
	class BS_D3D11_EXPORT D3D11OcclusionQuery : public OcclusionQuery
	{
	public:
		D3D11OcclusionQuery(bool binary);
		~D3D11OcclusionQuery();

		/** @copydoc OcclusionQuery::begin */
		void begin() override;

		/** @copydoc OcclusionQuery::end */
		void end() override;

		/** @copydoc OcclusionQuery::isReady */
		bool isReady() const override;

		/** @copydoc OcclusionQuery::getNumFragments */
		UINT32 getNumSamples() override;

	private:
		friend class QueryManager;

		/** Resolves query results after it is ready. */
		void finalize();

		ID3D11Query* mQuery;
		ID3D11DeviceContext* mContext;
		bool mFinalized;
		bool mQueryEndCalled;

		UINT32 mNumSamples;
	};

	/** @} */
}