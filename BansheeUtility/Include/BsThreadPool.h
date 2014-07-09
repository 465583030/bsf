#pragma once

#include "BsPrerequisitesUtil.h"
#include "BsModule.h"

namespace BansheeEngine
{
	class ThreadPool;

	/**
	 * @brief	Handle to a thread managed by ThreadPool.
	 */
	class BS_UTILITY_EXPORT HThread
	{
	public:
		HThread();
		HThread(ThreadPool* pool, UINT32 threadId);

		/**
		 * @brief	Block the calling thread until the thread this handle points to completes.
		 */
		void blockUntilComplete();

	private:
		UINT32 mThreadId;
		ThreadPool* mPool;
	};

	/**
	 * @brief	Wrapper around a thread that is used within ThreadPool.
	 */
	class BS_UTILITY_EXPORT PooledThread
	{
	public:
		PooledThread(const String& name);
		virtual ~PooledThread();

		/**
		 * @brief	Initializes the pooled thread. Must be called
		 * 			right after the object is constructed.
		 */
		void initialize();

		/**
		 * @brief	Starts executing the given worker method.
		 *
		 * @note	Caller must ensure worker method is not null and that the thread
		 * 			is currently idle, otherwise undefined behavior will occur.
		 */
		void start(std::function<void()> workerMethod);

		/**
		 * @brief	Attempts to join the currently running thread and destroys it. Caller must ensure
		 * 			that any worker method currently running properly returns, otherwise this
		 * 			will block indefinitely.
		 */
		void destroy();

		/**
		 * @brief	Returns true if the thread is idle and new worker method can be scheduled on it.
		 */
		bool isIdle();

		/**
		 * @brief	Returns how long has the thread been idle. Value is undefined if thread is not idle.
		 */
		time_t idleTime();

		/**
		 * @brief	Sets a name of the thread.
		 */
		void setName(const String& name);

		/**
		 * @brief	Gets unique ID of the currently executing thread.
		 */
		UINT32 getId() const;

		/**
		 * @brief	Called when the thread is first created.
		 */
		virtual void onThreadStarted(const String& name) = 0;

		/**
		 * @brief	Called when the thread is being shut down.
		 */
		virtual void onThreadEnded(const String& name) = 0;

	protected:
		friend class HThread;

		/**
		 * @brief	Primary worker method that is ran when the thread is first
		 * 			initialized.
		 */
		void run();

	protected:
		std::function<void()> mWorkerMethod;

		String mName;
		UINT32 mId;
		bool mIdle;
		bool mThreadStarted;
		bool mThreadReady;

		time_t mIdleTime;

		BS_THREAD_TYPE* mThread;
		BS_MUTEX(mMutex);
		BS_THREAD_SYNCHRONISER(mStartedCond);
		BS_THREAD_SYNCHRONISER(mReadyCond);
		BS_THREAD_SYNCHRONISER(mWorkerEndedCond);
	};

	/**
	 * @copydoc	PooledThread
	 * 			
	 * @tparam	ThreadPolicy Allows you specify a policy with methods that will get called
	 * 						 whenever a new thread is created or when a thread is destroyed.
	 */
	template<class ThreadPolicy>
	class TPooledThread : public PooledThread
	{
	public:
		TPooledThread(const String& name)
			:PooledThread(name)
		{ }

		/**
		 * @copydoc	PooledThread::onThreadStarted
		 */
		void onThreadStarted(const String& name)
		{
			ThreadPolicy::onThreadStarted(name);
		}

		/**
		 * @copydoc	PooledThread::onThreadEnded
		 */
		void onThreadEnded(const String& name)
		{
			ThreadPolicy::onThreadEnded(name);
		}
	};

	/**
	 * @brief	Class that maintains a pool of threads we can easily retrieve and use
	 * 			for any task. This saves on the cost of creating and destroying threads.
	 */
	class BS_UTILITY_EXPORT ThreadPool : public Module<ThreadPool>
	{
	public:
		/**
		 * @brief	Constructs a new thread pool
		 *
		 * @param	threadCapacity	Default thread capacity, the pool will always
		 * 							try to keep this many threads available.
		 * @param	maxCapacity   	(optional) Maximum number of threads the pool can create.
		 * 							If we go over this limit an exception will be thrown.
		 * @param	idleTimeout   	(optional) How many seconds do threads need to be idle before
		 * 							we remove them from the pool
		 */
		ThreadPool(UINT32 threadCapacity, UINT32 maxCapacity = 16, UINT32 idleTimeout = 60);
		virtual ~ThreadPool();

		/**
		 * @brief	Find an unused thread (or creates a new one) and runs the specified worker
		 * 			method on it.
		 *
		 * @param	name			A name you may use for more easily identifying the thread.
		 * @param	workerMethod	The worker method to be called by the thread.
		 *
		 * @returns	A thread handle you may use for monitoring the thread execution.
		 */
		HThread run(const String& name, std::function<void()> workerMethod);

		/**
		 * @brief	Stops all threads and destroys them. Caller must ensure each threads workerMethod
		 * 			returns otherwise this will never return.
		 */
		void stopAll();

		/**
		 * @brief	Clear any unused threads that are over the capacity.
		 */
		void clearUnused();

		/**
		 * @brief	Returns the number of unused threads in the pool.
		 */
		UINT32 getNumAvailable() const;

		/**
		 * @brief	Returns the number of running threads in the pool.
		 */
		UINT32 getNumActive() const;

		/**
		 * @brief	Returns the total number of created threads in the pool
		 * 			(both running and unused).
		 */
		UINT32 getNumAllocated() const;

	protected:
		friend class HThread;

		Vector<PooledThread*> mThreads;
		
		/**
		 * @brief	Creates a new thread to be used by the pool.
		 */
		virtual PooledThread* createThread(const String& name) = 0;

		/**
		 * @brief	Destroys the specified thread. Caller needs to make sure
		 * 			the thread is actually shut down beforehand.
		 */
		void destroyThread(PooledThread* thread);

		/**
		 * @brief	Returns the first unused thread if one exists, otherwise
		 * 			creates a new one.
		 *
		 * @param	name	Name to assign the thread.
		 *
		 * @note	Throws an exception if we have reached our maximum thread capacity.
		 */
		PooledThread* getThread(const String& name);

		UINT32 mDefaultCapacity;
		UINT32 mMaxCapacity;
		UINT32 mIdleTimeout;
		UINT32 mAge;
		
		BS_MUTEX(mMutex);
	};

	/**
	 * @brief	Policy used for thread start & end used by the ThreadPool.
	 */
	class ThreadNoPolicy
	{
	public:
		static void onThreadStarted(const String& name) { }
		static void onThreadEnded(const String& name) { }
	};

	/**
	 * @copydoc ThreadPool
	 * 			
	 * @tparam	ThreadPolicy Allows you specify a policy with methods that will get called
	 * 						 whenever a new thread is created or when a thread is destroyed.
	 */
	template<class ThreadPolicy = ThreadNoPolicy>
	class TThreadPool : public ThreadPool
	{
	public:
		TThreadPool(UINT32 threadCapacity, UINT32 maxCapacity = 16, UINT32 idleTimeout = 60)
			:ThreadPool(threadCapacity, maxCapacity, idleTimeout)
		{

		}

	protected:
		/**
		 * @copydoc ThreadPool::createThread
		 */
		PooledThread* createThread(const String& name)
		{
			PooledThread* newThread = bs_new<TPooledThread<ThreadPolicy>>(name);
			newThread->initialize();

			return newThread;
		}
	};
}