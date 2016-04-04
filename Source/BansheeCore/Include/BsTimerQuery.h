//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"

namespace BansheeEngine
{
	/** @addtogroup RenderAPI-Internal
	 *  @{
	 */

	/**
	 * Represents a GPU query that measures execution time of GPU operations. The query will measure any GPU operations 
	 * that take place between its begin() and end() calls.
	 * 			
	 * @note	Core thread only.
	 */
	class BS_CORE_EXPORT TimerQuery
	{
	public:
		virtual ~TimerQuery() {}

		/**
		 * Starts the counter. 
		 * 			
		 * @note	Place any commands you want to measure after this call. Call "end" when done.
		 */
		virtual void begin() = 0;

		/**	Stops the counter. */
		virtual void end() = 0;

		/**	Check if GPU has processed the query. */
		virtual bool isReady() const = 0;

		/**
		 * Returns the time it took for the query to execute.
		 *
		 * @return	The time milliseconds.
		 * 			
		 * @note	Only valid after isReady() returns true.
		 */
		virtual float getTimeMs() = 0;

		/** Triggered when GPU processes the query. As a parameter it provides query duration in milliseconds. */
		Event<void(float)> onTriggered;

		/**	Creates a new query, but does not schedule it on GPU. */
		static SPtr<TimerQuery> create();

	protected:
		friend class QueryManager;

		/**	Returns true if the has still not been completed by the GPU. */
		bool isActive() const { return mActive; }
		void setActive(bool active) { mActive = active; }

	protected:
		bool mActive;
	};

	/** @} */
}