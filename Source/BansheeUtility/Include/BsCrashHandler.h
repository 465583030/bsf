//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#define BS_MAX_STACKTRACE_DEPTH 200
#define BS_MAX_STACKTRACE_NAME_BYTES 1024

namespace BansheeEngine
{
	/** @addtogroup Internal-Utility
	 *  @{
	 */

	/** @addtogroup Error-Internal
	 *  @{
	 */

	/** Saves crash data and notifies the user when a crash occurs. */
	// TODO - Crashes are reported in the same process as the main application. This can be a problem if the crash was caused
	// by heap. Any further use of the heap by the reporting methods will cause a silent crash, failing to log it. A more appropriate
	// way of doing it should be to resume another process to actually handle the crash.
	class BS_UTILITY_EXPORT CrashHandler
	{
	public:
		CrashHandler();
		~CrashHandler();

		/** Constructs and starts the module. */
		static void startUp() { _instance() = bs_new<CrashHandler>(); }

		/** Shuts down this module and frees any resources it is using. */
		static void shutDown() { bs_delete(_instance()); }

		/** Returns a reference to the module instance. */
		static CrashHandler& instance() { return *_instance(); }

		/**
		 * Records a crash with a custom error message.
		 * 			
		 * @param[in]	type		Type of the crash that occurred. For example "InvalidParameter".
		 * @param[in]	description	More detailed description of the issue that caused the crash.
		 * @param[in]	function	Optional name of the function where the error occurred.
		 * @param[in]	file		Optional name of the source code file in which the code that crashed the program exists.
		 * @param[in]	line		Optional source code line at which the crash was triggered at.
		 */
		void reportCrash(const String& type, const String& description, const String& function = StringUtil::BLANK,
			const String& file = StringUtil::BLANK, UINT32 line = 0) const;

#if BS_PLATFORM == BS_PLATFORM_WIN32
		/**
		 * Records a crash resulting from a Windows-specific SEH exception. 
		 * 			
		 * @param[in]	exceptionData	Exception data returned from GetExceptionInformation()
		 * @return						Code that signals the __except exception handler on how to proceed.
		 *
		 * @note	Available in Windows builds only.
		 */
		int reportCrash(void* exceptionData) const;
#endif

		/**
		 * Returns a string containing a current stack trace. If function can be found in the symbol table its readable 
		 * name will be present in the stack trace, otherwise just its address.
		 * 						
		 * @return	String containing the call stack with each function on its own line.
		 */
		static String getStackTrace();
	private:
		/** Returns path to the folder into which to store the crash reports. */
		Path getCrashFolder() const;

		/** Returns a singleton instance of this module. */
		static CrashHandler*& _instance() { static CrashHandler* inst = nullptr; return inst; }

		static const wchar_t* CrashReportFolder;
		static const wchar_t* CrashLogName;

		struct Data;
		Data* m;
	};

	/** Easier way of accessing the CrashHandler. */
	BS_UTILITY_EXPORT CrashHandler& gCrashHandler();

	/** @} */
	/** @} */
}