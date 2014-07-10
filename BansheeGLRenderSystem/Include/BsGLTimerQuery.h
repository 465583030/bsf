//__________________________ Banshee Project - A modern game development toolkit _________________________________//
//_____________________________________ www.banshee-project.com __________________________________________________//
//________________________ Copyright (c) 2014 Marko Pintera. All rights reserved. ________________________________//
#pragma once

#include "BsGLPrerequisites.h"
#include "BsTimerQuery.h"

namespace BansheeEngine
{
	/**
	 * @brief	OpenGL implementation of a timer query.
	 */
	class BS_RSGL_EXPORT GLTimerQuery : public TimerQuery
	{
	public:
		GLTimerQuery();
		~GLTimerQuery();

		/**
		 * @copydoc TimerQuery::begin
		 */
		virtual void begin();

		/**
		 * @copydoc TimerQuery::end
		 */
		virtual void end();

		/**
		 * @copydoc TimerQuery::isReady
		 */
		virtual bool isReady() const;

		/**
		 * @copydoc TimerQuery::getTimeMs
		 */
		virtual float getTimeMs();

	private:
		friend class QueryManager;

		/**
		 * @brief	Processes query results and saves them for later use. To be called
		 *			when query has completed.
		 */
		void finalize();

	private:
		GLuint mQueryStartObj;
		GLuint mQueryEndObj;
		bool mFinalized;
		bool mEndIssued;

		float mTimeDelta;
	};
}