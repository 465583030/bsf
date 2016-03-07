//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsGLPrerequisites.h"
#include "BsOcclusionQuery.h"

namespace BansheeEngine
{
	/** @addtogroup GL
	 *  @{
	 */

	/**	OpenGL implementation of an occlusion query. */
	class BS_RSGL_EXPORT GLOcclusionQuery : public OcclusionQuery
	{
	public:
		GLOcclusionQuery(bool binary);
		~GLOcclusionQuery();

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

		/** Processes query results and saves them for later use. To be called when query has completed. */
		void finalize();

	private:
		GLuint mQueryObj;
		bool mFinalized;
		bool mEndIssued;

		UINT32 mNumSamples;
	};

	/** @} */
}