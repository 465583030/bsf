//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsDynLib.h"
#include "BsException.h"

#if BS_PLATFORM == BS_PLATFORM_WIN32
#  define WIN32_LEAN_AND_MEAN
#  if !defined(NOMINMAX) && defined(_MSC_VER)
#	define NOMINMAX // required to stop windows.h messing up std::min
#  endif
#  include <windows.h>
#endif

#if BS_PLATFORM == BS_PLATFORM_APPLE
#   include "macUtils.h"
#   include <dlfcn.h>
#endif

namespace BansheeEngine 
{
    DynLib::DynLib(const String& name)
    {
        mName = name;
        m_hInst = nullptr;

		load();
    }

    DynLib::~DynLib()
    {
    }

    void DynLib::load()
    {
		if(m_hInst)
			return;

		String name = mName;
#if BS_PLATFORM == BS_PLATFORM_LINUX
        // dlopen() does not add .so to the filename, like windows does for .dll
        if (name.substr(name.length() - 3, 3) != ".so")
           name += ".so";
#elif BS_PLATFORM == BS_PLATFORM_APPLE
        // dlopen() does not add .dylib to the filename, like windows does for .dll
        if (name.substr(name.length() - 6, 6) != ".dylib")
			name += ".dylib";
#elif BS_PLATFORM == BS_PLATFORM_WIN32
		// Although LoadLibraryEx will add .dll itself when you only specify the library name,
		// if you include a relative path then it does not. So, add it to be sure.
		if (name.substr(name.length() - 4, 4) != ".dll")
			name += ".dll";
#endif
        m_hInst = (DYNLIB_HANDLE)DYNLIB_LOAD(name.c_str());

        if(!m_hInst)
		{
            BS_EXCEPT(InternalErrorException,  
				"Could not load dynamic library " + mName + 
                ".  System Error: " + dynlibError());
		}
    }

    void DynLib::unload()
    {
		if(!m_hInst)
			return;

        if(DYNLIB_UNLOAD(m_hInst))
		{
			BS_EXCEPT(InternalErrorException, 
                "Could not unload dynamic library " + mName +
                ".  System Error: " + dynlibError());
		}
    }

    void* DynLib::getSymbol(const String& strName) const
    {
		if(!m_hInst)
			return nullptr;

        return (void*)DYNLIB_GETSYM(m_hInst, strName.c_str());
    }

    String DynLib::dynlibError() 
    {
#if BS_PLATFORM == BS_PLATFORM_WIN32
        LPVOID lpMsgBuf; 
        FormatMessage( 
            FORMAT_MESSAGE_ALLOCATE_BUFFER | 
            FORMAT_MESSAGE_FROM_SYSTEM | 
            FORMAT_MESSAGE_IGNORE_INSERTS, 
            NULL, 
            GetLastError(), 
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
            (LPTSTR) &lpMsgBuf, 
            0, 
            NULL 
            ); 
        String ret = (char*)lpMsgBuf;
        // Free the buffer.
        LocalFree(lpMsgBuf);
        return ret;
#elif BS_PLATFORM == BS_PLATFORM_LINUX || BS_PLATFORM == BS_PLATFORM_APPLE
        return String(dlerror());
#else
        return String("");
#endif
    }
}
