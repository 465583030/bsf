#pragma once

#include "BsPrerequisitesUtil.h"

namespace BansheeEngine
{
	/**
	 * @brief	Data common to all event connections.
	 */
	class BaseConnectionData
	{
	public:
		BaseConnectionData()
			:prev(nullptr), next(nullptr), isActive(true),
			handleLinks(0)
		{
			
		}

		virtual ~BaseConnectionData()
		{
			assert(!handleLinks && !isActive);
		}

		virtual void deactivate()
		{
			isActive = false;
		}

		BaseConnectionData* prev;
		BaseConnectionData* next;
		bool isActive;
		UINT32 handleLinks;
	};

	/**
	 * @brief	Internal data for an Event, storing all connections.
	 */
	struct EventInternalData
	{
		EventInternalData()
			:mConnections(nullptr), mFreeConnections(nullptr)
		{ }

		~EventInternalData()
		{
			BaseConnectionData* conn = mConnections;
			while (conn != nullptr)
			{
				BaseConnectionData* next = conn->next;
				bs_delete(conn);

				conn = next;
			}

			conn = mFreeConnections;
			while (conn != nullptr)
			{
				BaseConnectionData* next = conn->next;
				bs_delete(conn);

				conn = next;
			}
		}

		/**
		 * @brief	Disconnects the connection with the specified data,
		 *			ensuring the event doesn't call its callback again.
		 *
		 * @note	Only call this once.
		 */
		void disconnect(BaseConnectionData* conn)
		{
			BS_LOCK_RECURSIVE_MUTEX(mMutex);

			conn->deactivate();
			conn->handleLinks--;

			if (conn->handleLinks == 0)
				free(conn);
		}

		/**
		 * @brief	Disconnects all connections in the event.
		 */
		void clear()
		{
			BS_LOCK_RECURSIVE_MUTEX(mMutex);

			BaseConnectionData* conn = mConnections;
			while (conn != nullptr)
			{
				BaseConnectionData* next = conn->next;
				conn->deactivate();

				if (conn->handleLinks == 0)
					free(conn);

				conn = next;
			}
		}

		/**
		 * @brief	Called when the event handle no longer keeps
		 *			a reference to the connection data. This means
		 *			we might be able to free (and reuse) its memory
		 *			if the event is done with it too.
		 */
		void freeHandle(BaseConnectionData* conn)
		{
			BS_LOCK_RECURSIVE_MUTEX(mMutex);

			conn->handleLinks--;

			if (conn->handleLinks == 0 && !conn->isActive)
				free(conn);
		}

		/**
		 * @brief	Releases connection data and makes it
		 *			available for re-use when next connection
		 *			is formed.
		 */
		void free(BaseConnectionData* conn)
		{
			if (conn->prev != nullptr)
				conn->prev->next = conn->next;
			else
				mConnections = conn->next;

			if (conn->next != nullptr)
				conn->next->prev = conn->prev;

			conn->prev = nullptr;
			conn->next = nullptr;

			if (mFreeConnections != nullptr)
			{
				conn->next = mFreeConnections;
				mFreeConnections->prev = conn;
			}

			mFreeConnections = conn;
		}

		BaseConnectionData* mConnections;
		BaseConnectionData* mFreeConnections;

		BS_RECURSIVE_MUTEX(mMutex);
	};

	/**
	 * @brief	Event handle. Allows you to track to which events you subscribed to and
	 *			disconnect from them when needed.
	 */
	class HEvent
	{
	public:
		HEvent()
			:mConnection(nullptr)
		{ }

		explicit HEvent(const SPtr<EventInternalData>& eventData, BaseConnectionData* connection)
			:mConnection(connection), mEventData(eventData)
		{
			connection->handleLinks++;
		}

		~HEvent()
		{
			if (mConnection != nullptr)
				mEventData->freeHandle(mConnection);
		}

		/**
		 * @brief	Disconnect from the event you are subscribed to.
		 */
		void disconnect()
		{
			if (mConnection != nullptr)
			{
				mEventData->disconnect(mConnection);
				mConnection = nullptr;
			}
		}

		struct Bool_struct
		{
			int _Member;
		};

		/**
		* @brief	Allows direct conversion of a handle to bool.
		*
		* @note		This is needed because we can't directly convert to bool
		*			since then we can assign pointer to bool and that's wrong.
		*/
		operator int Bool_struct::*() const
		{
			return (mConnection != nullptr ? &Bool_struct::_Member : 0);
		}

		HEvent& operator=(const HEvent& rhs)
		{
			mConnection = rhs.mConnection;
			mEventData = rhs.mEventData;

			if (mConnection != nullptr)
				mConnection->handleLinks++;

			return *this;
		}

	private:
		BaseConnectionData* mConnection;
		SPtr<EventInternalData> mEventData;
	};	

	/**
	 * @brief	Events allows you to register method callbacks that get notified
	 *			when the event is triggered.
	 *
	 * @note	Callback method return value is ignored.
	 */
	// Note: I could create a policy template argument that allows creation of 
	// lockable and non-lockable events in the case mutex is causing too much overhead.
	template <class RetType, class... Args>
	class TEvent
	{
		struct ConnectionData : BaseConnectionData
		{
		public:
			void deactivate() override
			{
				func = nullptr;

				BaseConnectionData::deactivate();
			}

			std::function<RetType(Args...)> func;
		};

	public:
		TEvent()
			:mInternalData(bs_shared_ptr<EventInternalData>())
		{ }

		~TEvent()
		{
			clear();
		}

		/**
		 * @brief	Register a new callback that will get notified once
		 *			the event is triggered.
		 */
		HEvent connect(std::function<RetType(Args...)> func)
		{
			BS_LOCK_RECURSIVE_MUTEX(mInternalData->mMutex);

			ConnectionData* connData = nullptr;
			if (mInternalData->mFreeConnections != nullptr)
			{
				connData = static_cast<ConnectionData*>(mInternalData->mFreeConnections);
				mInternalData->mFreeConnections = connData->next;

				if (connData->next != nullptr)
					connData->next->prev = nullptr;

				connData->isActive = true;
				connData->handleLinks = true;
			}

			if (connData == nullptr)
				connData = bs_new<ConnectionData>();

			connData->next = mInternalData->mConnections;

			if (mInternalData->mConnections != nullptr)
				mInternalData->mConnections->prev = connData;

			mInternalData->mConnections = connData;
			connData->func = func;

			return HEvent(mInternalData, connData);
		}

		/**
		 * @brief	Trigger the event, notifying all register callback methods.
		 */
		void operator() (Args... args)
		{
			// Increase ref count to ensure this event data isn't destroyed if one of the callbacks
			// deletes the event itself.
			std::shared_ptr<EventInternalData> internalData = mInternalData;

			BS_LOCK_RECURSIVE_MUTEX(internalData->mMutex);

			// Hidden dependency: If any new connections are made during these callbacks they must be
			// inserted at the start of the linked list so that we don't trigger them here.
			ConnectionData* conn = static_cast<ConnectionData*>(mInternalData->mConnections);
			while (conn != nullptr)
			{
				// Save next here in case the callback itself disconnects this connection
				ConnectionData* next = static_cast<ConnectionData*>(conn->next);
				
				if (conn->func != nullptr)
					conn->func(args...);

				conn = next;
			}
		}

		/**
		 * @brief	Clear all callbacks from the event.
		 */
		void clear()
		{
			mInternalData->clear();
		}

		/**
		 * @brief	Check if event has any callbacks registered.
		 *
		 * @note	It is safe to trigger an event even if no callbacks are registered.
		 */
		bool empty()
		{
			BS_LOCK_RECURSIVE_MUTEX(mInternalData->mMutex);

			return mInternalData->mConnections == nullptr;
		}

	private:
		SPtr<EventInternalData> mInternalData;
	};

	/************************************************************************/
	/* 							SPECIALIZATIONS                      		*/
	/* 	SO YOU MAY USE FUNCTION LIKE SYNTAX FOR DECLARING EVENT SIGNATURE   */
	/************************************************************************/
	
	/**
	 * @copydoc	TEvent
	 */
	template <typename Signature>
	class Event;

	/**
	* @copydoc	TEvent
	*/
	template <class RetType, class... Args>
	class Event<RetType(Args...) > : public TEvent <RetType, Args...>
	{ };
}