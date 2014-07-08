#pragma once

#include "BsCorePrerequisites.h"
#include "BsEventQuery.h"
#include "BsModule.h"

namespace BansheeEngine
{
	/**
	 * @brief	Handles creation and destruction of GPU queries.
	 * 			
	 * @note	Core thread only.
	 */
	class BS_CORE_EXPORT QueryManager : public Module<QueryManager>
	{
	public:
		QueryManager();
		~QueryManager();

		/**
		 * @brief	Creates a new event query that allows you to get notified
		 *			when GPU starts executing the query.
		 */
		virtual EventQueryPtr createEventQuery() const = 0;

		/**
		* @brief	Creates a new timer query that allows you to get notified
		*			of how much time has passed between query start and end.
		*/
		virtual TimerQueryPtr createTimerQuery() const = 0;

		/**
		* @brief	Creates a new occlusion query that allows you to know
		*			how many fragments were rendered between query start and end.
		*
		* @param binary	If query is binary it will not give you an exact count of fragments rendered, but will instead
		*				just return 0 (no fragments were rendered) or 1 (one or more fragments were rendered). Binary
		*				queries can return sooner as they potentially do not need to wait until all of the geometry is rendered.
		*/
		virtual OcclusionQueryPtr createOcclusionQuery(bool binary) const = 0;

		/**
		 * @note	Internal method, called every frame.
		 */
		void _update();

	protected:
		friend class EventQuery;
		friend class TimerQuery;
		friend class OcclusionQuery;

		/**
		 * @brief	Deletes an Event query. Always use this method and don't delete them manually.
		 */
		static void deleteEventQuery(EventQuery* query);

		/**
		* @brief	Deletes a Timer query. Always use this method and don't delete them manually.
		*/
		static void deleteTimerQuery(TimerQuery* query);

		/**
		* @brief	Deletes an Occlusion query. Always use this method and don't delete them manually.
		*/
		static void deleteOcclusionQuery(OcclusionQuery* query);

	protected:
		mutable Vector<EventQuery*> mEventQueries;
		mutable Vector<TimerQuery*> mTimerQueries;
		mutable Vector<OcclusionQuery*> mOcclusionQueries;
	};
}