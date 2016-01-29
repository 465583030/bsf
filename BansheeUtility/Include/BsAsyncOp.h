//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisitesUtil.h"
#include "BsException.h"
#include "BsAny.h"

namespace BansheeEngine
{
	/** @addtogroup Threading
	 *  @{
	 */

	/** Thread synchronization primitives used by AsyncOps and their callers. */
	class BS_UTILITY_EXPORT AsyncOpSyncData
	{
	public:
		BS_MUTEX(mMutex)
		BS_THREAD_SYNCHRONISER(mCondition)
	};

	/**
	 * Flag used for creating async operations signaling that we want to create an empty AsyncOp with no internal 
	 * memory storage.
	 */
	struct BS_UTILITY_EXPORT AsyncOpEmpty {};

	/**
	 * Object you may use to check on the results of an asynchronous operation. Contains uninitialized data until 
	 * hasCompleted() returns true. 
	 * 			
	 * @note	
	 * You are allowed (and meant to) to copy this by value.
	 * @note
	 * You'll notice mIsCompleted isn't synchronized. This is because we're okay if mIsCompleted reports true a few cycles 
	 * too late, which is not relevant for practical use. And in cases where you need to ensure operation has completed 
	 * you will usually use some kind of synchronization primitive that includes a memory barrier anyway.
	 */
	class BS_UTILITY_EXPORT AsyncOp
	{
	private:
		struct AsyncOpData
		{
			AsyncOpData()
				:mIsCompleted(false)
			{ }

			Any mReturnValue;
			volatile std::atomic<bool> mIsCompleted;
		};

	public:
		AsyncOp()
			:mData(bs_shared_ptr_new<AsyncOpData>())
		{ }

		AsyncOp(AsyncOpEmpty empty)
		{ }

		AsyncOp(const AsyncOpSyncDataPtr& syncData)
			:mData(bs_shared_ptr_new<AsyncOpData>()), mSyncData(syncData)
		{ }

		AsyncOp(AsyncOpEmpty empty, const AsyncOpSyncDataPtr& syncData)
			:mSyncData(syncData)
		{ }

		/** True if the async operation has completed. */
		bool hasCompleted() const;

		/**
		 * Blocks the caller thread until the AsyncOp completes.
		 *
		 * @note
		 * Do not call this on the thread that is completing the async op, as it will cause a deadlock. Make sure the 
		 * command you are waiting for is actually queued for execution because a deadlock will occurr otherwise.
		 */
		void blockUntilComplete() const;

		/** Retrieves the value returned by the async operation. Only valid if hasCompleted() returns true. */
		template <typename T>
		T getReturnValue() const 
		{ 
#if BS_DEBUG_MODE
			if(!hasCompleted())
				BS_EXCEPT(InternalErrorException, "Trying to get AsyncOp return value but the operation hasn't completed.");
#endif
			// Be careful if cast throws an exception. It doesn't support casting of polymorphic types. Provided and returned
			// types must be EXACT. (You'll have to cast the data yourself when completing the operation)
			return any_cast<T>(mData->mReturnValue);
		}

	public: // ***** INTERNAL ******
		/** @cond INTERNAL */

		/**
		 * Mark the async operation as completed.
		 *
		 * @note	Internal method.
		 */
		void _completeOperation(Any returnValue);

		/**
		 * Mark the async operation as completed, without setting a return value.
		 *
		 * @note	Internal method.
		 */
		void _completeOperation();

		/** @endcond */
	private:
		std::shared_ptr<AsyncOpData> mData;
		AsyncOpSyncDataPtr mSyncData;
	};

	/** @} */
}