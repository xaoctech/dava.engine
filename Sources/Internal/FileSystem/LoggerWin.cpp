#include "FileSystem/Logger.h"

#if defined(__DAVAENGINE_WIN32__)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>

namespace DAVA 
{

void Logger::PlatformLog(eLogLevel ll, const char8* text)
{
	OutputDebugStringA(text);
	printf("%s", text);
}

void Logger::PlatformLog(eLogLevel ll, const char16* text)
{
	OutputDebugStringW(text);
	wprintf(L"%s", text);
}

}

#endif 
