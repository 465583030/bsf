#define WIN32_LEAN_AND_MEAN
#if !defined(NOMINMAX) && defined(_MSC_VER)
#	define NOMINMAX // Required to stop windows.h messing up std::min
#endif
#include <windows.h>
#include <windowsx.h>
#include <oleidl.h>

#define WM_BS_SETCAPTURE WM_USER + 101
#define WM_BS_RELEASECAPTURE WM_USER + 102