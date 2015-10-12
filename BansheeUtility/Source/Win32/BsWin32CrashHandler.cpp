#include "BsCrashHandler.h"
#include "BsDebug.h"
#include "BsDynLib.h"
#include "BsFileSystem.h"
#include "DbgHelp.h"
#include <psapi.h>

namespace BansheeEngine
{
	/**
	 * @brief	Returns the raw stack trace using the provided context. Raw stack trace contains only
	 * 			function addresses.
	 * 			
	 * @param	context		Processor context from which to start the stack trace. 
	 * @param	stackTrace	Output parameter that will contain the function addresses. First address is the deepest
	 * 						called function and following address is its caller and so on.
	 * 						
	 * @returns	Number of functions in the call stack.
	 */
	UINT32 win32_getRawStackTrace(CONTEXT context, UINT64 stackTrace[BS_MAX_STACKTRACE_DEPTH])
	{
		HANDLE hProcess = GetCurrentProcess();
		HANDLE hThread = GetCurrentThread();
		UINT32 machineType;

		STACKFRAME64 stackFrame;
		memset(&stackFrame, 0, sizeof(stackFrame));

		stackFrame.AddrPC.Mode = AddrModeFlat;
		stackFrame.AddrStack.Mode = AddrModeFlat;
		stackFrame.AddrFrame.Mode = AddrModeFlat;

#if BS_ARCH_TYPE == BS_ARCHITECTURE_x86_64
		stackFrame.AddrPC.Offset = context.Rip;
		stackFrame.AddrStack.Offset = context.Rsp;
		stackFrame.AddrFrame.Offset = context.Rbp;

		machineType = IMAGE_FILE_MACHINE_AMD64;
#else
		stackFrame.AddrPC.Offset = context.Eip;
		stackFrame.AddrStack.Offset = context.Esp;
		stackFrame.AddrFrame.Offset = context.Ebp;

		machineType = IMAGE_FILE_MACHINE_I386;
#endif

		UINT32 numEntries = 0;
		while (true)
		{
			if (!StackWalk64(machineType, hProcess, hThread, &stackFrame, &context, nullptr,
				SymFunctionTableAccess64, SymGetModuleBase64, nullptr))
			{
				LOGERR("Stack walk failed.");
				break;
			}

			if (numEntries < BS_MAX_STACKTRACE_DEPTH)
				stackTrace[numEntries] = stackFrame.AddrPC.Offset;

			numEntries++;

			if (stackFrame.AddrPC.Offset == 0 || stackFrame.AddrFrame.Offset == 0)
				break;
		}

		return numEntries;
	}

	/**
	 * @brief	Returns a string containing a stack trace using the provided context. If function can be found in the symbol
	 * 			table its readable name will be present in the stack trace, otherwise just its address.
	 * 			
	 * @param	context		Processor context from which to start the stack trace. 
	 * @param	skip		Number of bottom-most call stack entries to skip.
	 * 						
	 * @returns	String containing the call stack with each function on its own line.
	 */
	String win32_getStackTrace(CONTEXT context, int skip = 0)
	{
		UINT64 rawStackTrace[BS_MAX_STACKTRACE_DEPTH];
		UINT32 numEntries = win32_getRawStackTrace(context, rawStackTrace);

		numEntries = std::min((UINT32)BS_MAX_STACKTRACE_DEPTH, numEntries);

		UINT32 bufferSize = sizeof(PIMAGEHLP_SYMBOL64) + BS_MAX_STACKTRACE_NAME_BYTES;
		UINT8* buffer = (UINT8*)bs_alloc(bufferSize);

		PIMAGEHLP_SYMBOL64 symbol = (PIMAGEHLP_SYMBOL64)buffer;
		symbol->SizeOfStruct = bufferSize;
		symbol->MaxNameLength = BS_MAX_STACKTRACE_NAME_BYTES;

		HANDLE hProcess = GetCurrentProcess();

		StringStream outputStream;
		for (UINT32 i = skip; i < numEntries; i++)
		{
			if (i > skip)
				outputStream << std::endl;

			DWORD64 funcAddress = rawStackTrace[i];

			// Output function name
			DWORD64 dummy;
			if (SymGetSymFromAddr64(hProcess, funcAddress, &dummy, symbol))
				outputStream << StringUtil::format("{0}() - ", symbol->Name);

			// Output file name and line
			IMAGEHLP_LINE64	lineData;
			lineData.SizeOfStruct = sizeof(lineData);

			String addressString = toString(funcAddress, 0, ' ', std::ios::hex);

			DWORD column;
			if (SymGetLineFromAddr64(hProcess, funcAddress, &column, &lineData))
			{
				outputStream << StringUtil::format("0x{0} File[{1}:{2} ({3})]", addressString, 
					lineData.FileName, lineData.LineNumber, column);
			}
			else
			{
				outputStream << StringUtil::format("0x{0}", addressString);
			}

			// Output module name
			IMAGEHLP_MODULE64 moduleData;
			moduleData.SizeOfStruct = sizeof(moduleData);

			if (SymGetModuleInfo64(hProcess, funcAddress, &moduleData))
				outputStream << StringUtil::format(" Module[{0}]", moduleData.ImageName);
		}

		bs_free(buffer);

		return outputStream.str();
	}

	typedef bool(WINAPI *EnumProcessModulesType)(HANDLE hProcess, HMODULE* lphModule, DWORD cb, LPDWORD lpcbNeeded);
	typedef DWORD(WINAPI *GetModuleBaseNameType)(HANDLE hProcess, HMODULE hModule, LPSTR lpBaseName, DWORD nSize);
	typedef DWORD(WINAPI *GetModuleFileNameExType)(HANDLE hProcess, HMODULE hModule, LPSTR lpFilename, DWORD nSize);
	typedef bool(WINAPI *GetModuleInformationType)(HANDLE hProcess, HMODULE hModule, LPMODULEINFO lpmodinfo, DWORD cb);

	static DynLib* gPSAPILib = nullptr;

	static EnumProcessModulesType gEnumProcessModules;
	static GetModuleBaseNameType gGetModuleBaseName;
	static GetModuleFileNameExType gGetModuleFileNameEx;
	static GetModuleInformationType gGetModuleInformation;

	/**
	 * @brief	Dynamically load the PSAPI.dll and the required symbols, if not already loaded.
	 */
	void win32_initPSAPI()
	{
		if (gPSAPILib != nullptr)
			return;

		gPSAPILib = bs_new<DynLib>("PSAPI.dll");
		gEnumProcessModules = (EnumProcessModulesType)gPSAPILib->getSymbol("EnumProcessModules");
		gGetModuleBaseName = (GetModuleBaseNameType)gPSAPILib->getSymbol("GetModuleFileNameExA");
		gGetModuleFileNameEx = (GetModuleFileNameExType)gPSAPILib->getSymbol("GetModuleBaseNameA");
		gGetModuleInformation = (GetModuleInformationType)gPSAPILib->getSymbol("GetModuleInformation");
	}

	/**
	 * @brief	Unloads the PSAPI.dll if is loaded.
	 */
	void win32_unloadPSAPI()
	{
		if (gPSAPILib == nullptr)
			return;

		gPSAPILib->unload();
		bs_delete(gPSAPILib);
		gPSAPILib = nullptr;
	}

	static bool gSymbolsLoaded = false;

	/**
	 * @brief	Loads symbols for all modules in the current process. Loaded symbols allow the stack walker to retrieve
	 * 			human readable method, file, module names and other information.
	 */
	void win32_loadSymbols()
	{
		if (gSymbolsLoaded)
			return;

		HANDLE hProcess = GetCurrentProcess();
		UINT32 options = SymGetOptions();

		options |= SYMOPT_LOAD_LINES;
		options |= SYMOPT_EXACT_SYMBOLS;
		options |= SYMOPT_UNDNAME;
		options |= SYMOPT_FAIL_CRITICAL_ERRORS;
		options |= SYMOPT_NO_PROMPTS;
		options |= SYMOPT_DEFERRED_LOADS;

		SymSetOptions(options);
		SymInitialize(hProcess, nullptr, true);

		DWORD bufferSize;
		gEnumProcessModules(hProcess, nullptr, 0, &bufferSize);

		HMODULE* modules = (HMODULE*)bs_alloc(bufferSize);
		gEnumProcessModules(hProcess, modules, bufferSize, &bufferSize);

		UINT32 numModules = bufferSize / sizeof(HMODULE);
		for (UINT32 i = 0; i < numModules; i++)
		{
			MODULEINFO moduleInfo;

			char moduleName[BS_MAX_STACKTRACE_NAME_BYTES];
			char imageName[BS_MAX_STACKTRACE_NAME_BYTES];

			gGetModuleInformation(hProcess, modules[i], &moduleInfo, sizeof(moduleInfo));
			gGetModuleFileNameEx(hProcess, modules[i], imageName, BS_MAX_STACKTRACE_NAME_BYTES);
			gGetModuleBaseName(hProcess, modules[i], moduleName, BS_MAX_STACKTRACE_NAME_BYTES);

			char pdbSearchPath[BS_MAX_STACKTRACE_NAME_BYTES];
			GetFullPathNameA(imageName, BS_MAX_STACKTRACE_NAME_BYTES, pdbSearchPath, nullptr);
			SymSetSearchPath(GetCurrentProcess(), pdbSearchPath);

			SymLoadModule64(hProcess, modules[i], imageName, moduleName, (DWORD64)moduleInfo.lpBaseOfDll,
				(DWORD)moduleInfo.SizeOfImage);
		}

		bs_free(modules);
		gSymbolsLoaded = true;
	}

	/**
	 * @brief	Converts an exception record into a human readable error message.
	 */
	String win32_getExceptionMessage(EXCEPTION_RECORD* record)
	{
		String exceptionAddress = toString((UINT64)record->ExceptionAddress, 0, ' ', std::ios::hex);

		String format;
		switch (record->ExceptionCode)
		{
		case EXCEPTION_ACCESS_VIOLATION:
			{
				DWORD_PTR violatedAddress = 0;
				if (record->NumberParameters == 2)
				{
					if (record->ExceptionInformation[0] == 0)
						format = "Unhandled exception at 0x{0}. Access violation reading 0x{1}.";
					else if (record->ExceptionInformation[0] == 8)
						format = "Unhandled exception at 0x{0}. Access violation DEP 0x{1}.";
					else
						format = "Unhandled exception at 0x{0}. Access violation writing 0x{1}.";

					violatedAddress = record->ExceptionInformation[1];
				}
				else
					format = "Unhandled exception at 0x{0}. Access violation.";

				String violatedAddressStr = toString(violatedAddress, 0, ' ', std::ios::hex);
				return StringUtil::format(format, exceptionAddress, violatedAddressStr);
			}
		case EXCEPTION_IN_PAGE_ERROR:
		{
			DWORD_PTR violatedAddress = 0;
			DWORD_PTR code = 0;
			if (record->NumberParameters == 3)
			{
				if (record->ExceptionInformation[0] == 0)
					format = "Unhandled exception at 0x{0}. Page fault reading 0x{1} with code 0x{2}.";
				else if (record->ExceptionInformation[0] == 8)
					format = "Unhandled exception at 0x{0}. Page fault DEP 0x{1} with code 0x{2}.";
				else
					format = "Unhandled exception at 0x{0}. Page fault writing 0x{1} with code 0x{2}.";

				violatedAddress = record->ExceptionInformation[1];
				code = record->ExceptionInformation[3];
			}
			else
				format = "Unhandled exception at 0x{0}. Page fault.";

			String violatedAddressStr = toString(violatedAddress, 0, ' ', std::ios::hex);
			String codeStr = toString(code, 0, ' ', std::ios::hex);
			return StringUtil::format(format, exceptionAddress, violatedAddressStr, codeStr);
		}
		case STATUS_ARRAY_BOUNDS_EXCEEDED:
		{
			format = "Unhandled exception at 0x{0}. Attempting to access an out of range array element.";
			return StringUtil::format(format, exceptionAddress);
		}
		case EXCEPTION_DATATYPE_MISALIGNMENT:
		{
			format = "Unhandled exception at 0x{0}. Attempting to access missaligned data.";
			return StringUtil::format(format, exceptionAddress);
		}
		case EXCEPTION_FLT_DENORMAL_OPERAND:
		{
			format = "Unhandled exception at 0x{0}. Floating point operand too small.";
			return StringUtil::format(format, exceptionAddress);
		}
		case EXCEPTION_FLT_DIVIDE_BY_ZERO:
		{
			format = "Unhandled exception at 0x{0}. Floating point operation attempted to divide by zero.";
			return StringUtil::format(format, exceptionAddress);
		}
		case EXCEPTION_FLT_INVALID_OPERATION:
		{
			format = "Unhandled exception at 0x{0}. Floating point invalid operation.";
			return StringUtil::format(format, exceptionAddress);
		}
		case EXCEPTION_FLT_OVERFLOW:
		{
			format = "Unhandled exception at 0x{0}. Floating point overflow.";
			return StringUtil::format(format, exceptionAddress);
		}
		case EXCEPTION_FLT_UNDERFLOW:
		{
			format = "Unhandled exception at 0x{0}. Floating point underflow.";
			return StringUtil::format(format, exceptionAddress);
		}
		case EXCEPTION_FLT_STACK_CHECK:
		{
			format = "Unhandled exception at 0x{0}. Floating point stack overflow/underflow.";
			return StringUtil::format(format, exceptionAddress);
		}
		case EXCEPTION_ILLEGAL_INSTRUCTION:
		{
			format = "Unhandled exception at 0x{0}. Attempting to execute an illegal instruction.";
			return StringUtil::format(format, exceptionAddress);
		}
		case EXCEPTION_PRIV_INSTRUCTION:
		{
			format = "Unhandled exception at 0x{0}. Attempting to execute a private instruction.";
			return StringUtil::format(format, exceptionAddress);
		}
		case EXCEPTION_INT_DIVIDE_BY_ZERO:
		{
			format = "Unhandled exception at 0x{0}. Integer operation attempted to divide by zero.";
			return StringUtil::format(format, exceptionAddress);
		}
		case EXCEPTION_INT_OVERFLOW:
		{
			format = "Unhandled exception at 0x{0}. Integer operation result has overflown.";
			return StringUtil::format(format, exceptionAddress);
		}
		case EXCEPTION_STACK_OVERFLOW:
		{
			format = "Unhandled exception at 0x{0}. Stack overflow.";
			return StringUtil::format(format, exceptionAddress);
		}
		default:
		{
			format = "Unhandled exception at 0x{0}. Code 0x{1}.";

			String exceptionCode = toString((UINT32)record->ExceptionCode, 0, ' ', std::ios::hex);
			return StringUtil::format(format, exceptionAddress, exceptionCode);
		}
		}
	}

	void win32_writeMiniDump(const Path& filePath, EXCEPTION_POINTERS* exceptionData)
	{
		HANDLE hFile = CreateFileW(filePath.toWString().c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, 
			FILE_ATTRIBUTE_NORMAL, nullptr);

		if (hFile != INVALID_HANDLE_VALUE)
		{
			MINIDUMP_EXCEPTION_INFORMATION DumpExceptionInfo;

			DumpExceptionInfo.ThreadId = GetCurrentThreadId();
			DumpExceptionInfo.ExceptionPointers = exceptionData;
			DumpExceptionInfo.ClientPointers = false;

			MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, 
				&DumpExceptionInfo, nullptr, nullptr);
			CloseHandle(hFile);
		}
	}

	static const wchar_t* gMiniDumpName = L"minidump.dmp";
	const wchar_t* CrashHandler::CrashReportFolder = L"CrashReport-{0}/";
	const wchar_t* CrashHandler::CrashLogName = L"log.html";

	struct CrashHandler::Data
	{
		Mutex mutex;
	};

	CrashHandler::CrashHandler()
	{
		m = bs_new<Data>();
	}

	CrashHandler::~CrashHandler()
	{
		win32_unloadPSAPI();

		bs_delete(m);
	}

	void CrashHandler::reportCrash(const char* type, const char* description, const char* function,
		const char* file, UINT32 line)
	{
		// Win32 debug methods are not thread safe
		Lock<>(m->mutex);

		String stackTrace = getStackTrace();

		StringStream errorMessageStream;
		errorMessageStream << "Fatal error occurred in Banshee Engine and the program has to terminate!" << std::endl;
		errorMessageStream << "\t\t" << type << " - " << description << std::endl;
		errorMessageStream << "\t\t in " << function << " [" << file << ":" << line << "]" << std::endl;
		errorMessageStream << std::endl;
		errorMessageStream << "Stack trace: " << std::endl;
		errorMessageStream << stackTrace;

		String errorMessage = errorMessageStream.str();
		gDebug().logError(errorMessage);

		Path crashFolder = getCrashFolder();
		FileSystem::createDir(crashFolder);

		gDebug().getLog().saveToFile(crashFolder + toWString(CrashLogName));
		win32_writeMiniDump(crashFolder + toWString(gMiniDumpName), nullptr);

		MessageBoxA(nullptr, errorMessage.c_str(), "Banshee fatal error!", MB_OK);

		// Note: Potentially also log Windows Error Report and/or send crash data to server
	}

	int CrashHandler::reportCrash(EXCEPTION_POINTERS* exceptionData)
	{
		// Win32 debug methods are not thread safe
		Lock<>(m->mutex);

		win32_initPSAPI();
		win32_loadSymbols();
		String stackTrace = win32_getStackTrace(*exceptionData->ContextRecord, 0);

		StringStream errorMessageStream;
		errorMessageStream << "Fatal error occurred in Banshee Engine and the program has to terminate!" << std::endl;
		errorMessageStream << "\t\t" << win32_getExceptionMessage(exceptionData->ExceptionRecord);
		errorMessageStream << std::endl;
		errorMessageStream << "Stack trace: " << std::endl;
		errorMessageStream << stackTrace;

		String errorMessage = errorMessageStream.str();
		gDebug().logError(errorMessage);

		Path crashFolder = getCrashFolder();
		FileSystem::createDir(crashFolder);

		gDebug().getLog().saveToFile(crashFolder + toWString(CrashLogName));
		win32_writeMiniDump(crashFolder + toWString(gMiniDumpName), exceptionData);

		MessageBoxA(nullptr, errorMessage.c_str(), "Banshee fatal error!", MB_OK);

		// Note: Potentially also log Windows Error Report and/or send crash data to server

		return EXCEPTION_EXECUTE_HANDLER;
	}

	Path CrashHandler::getCrashFolder()
	{
		SYSTEMTIME systemTime;
		GetLocalTime(&systemTime);

		WString timeStamp = L"{0}-{1}-{2}_{3}:{4}";
		timeStamp = StringUtil::format(timeStamp, systemTime.wYear, systemTime.wMonth, systemTime.wDay, 
			systemTime.wHour, systemTime.wMinute);

		WString folderName = StringUtil::format(CrashReportFolder, timeStamp);

		return FileSystem::getWorkingDirectoryPath() + folderName;
	}

	String CrashHandler::getStackTrace()
	{
		CONTEXT context;
		RtlCaptureContext(&context);

		win32_initPSAPI();
		win32_loadSymbols();
		return win32_getStackTrace(context, 0);
	}
}