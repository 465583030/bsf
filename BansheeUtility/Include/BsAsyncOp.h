#pragma once

#include "BsPrerequisitesUtil.h"
#include "BsException.h"
#include "BsAny.h"

namespace BansheeEngine
{
	/**
	 * @brief	Object you may use to check on the results of an asynchronous operation. 
	 *			Contains uninitialized data until "hasCompleted" returns true. 
	 * 			
	 * @note	You are allowed (and meant to) to copy this by value.
	 * 			
	 *			You'll notice mIsCompleted isn't synchronized. This is because we're okay if 
	 *			mIsCompleted reports true a few cycles too late, which is not relevant for practical use.
	 *			And in cases where you need to ensure operation has completed you will usually use some kind
	 *			of synchronization primitive that includes a memory barrier anyway.
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
			volatile bool mIsCompleted;
		};

	public:
		AsyncOp()
			:mData(bs_shared_ptr<AsyncOpData, ScratchAlloc>())
		{
#if BS_ARCH_TYPE != BS_ARCHITECTURE_x86_32 && BS_ARCH_TYPE != BS_ARCHITECTURE_x86_64
			// On some architectures it is not guaranteed that stores are executed in order, which means
			// return value could be stored after mIsCompleted is set
			static_assert(false, "You will likely need to add a memory barrier for mIsCompleted on architectures other than x86.");
#endif
		}

		/**
		 * @brief	True if the async operation has completed.
		 */
		bool hasCompleted() const { return mData->mIsCompleted; }

		/**
		 * @brief	Internal method. Mark the async operation as completed.
		 */
		void _completeOperation(Any returnValue);

		/**
		 * @brief	Internal method. Mark the async operation as completed, without setting a return value.
		 */
		void _completeOperation();

		/**
		 * @brief	Retrieves the value returned by the async operation. Only valid
		 *			if "hasCompleted" returns true.
		 */
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

	private:
		std::shared_ptr<AsyncOpData> mData;
	};
}