#include "FileSystem/Logger.h"

#if defined(__DAVAENGINE_ANDROID__)

#include <stdarg.h>
#include <android/log.h>
#include "Utils/StringFormat.h"

namespace DAVA 
{

static DAVA::String androidLogTag = "";    

int32 LogLevelToAndtoid(Logger::eLogLevel ll)
{
	int32 androidLL = ANDROID_LOG_DEFAULT;
	switch (ll)
	{
		case Logger::LEVEL_DEBUG:
			androidLL = ANDROID_LOG_DEBUG;
			break;

		case Logger::LEVEL_INFO:
			androidLL = ANDROID_LOG_INFO;
			break;

		case Logger::LEVEL_WARNING:
			androidLL = ANDROID_LOG_WARN;
			break;

		case Logger::LEVEL_ERROR:
			androidLL = ANDROID_LOG_ERROR;
			break;
	}

	return androidLL;
}

void Logger::PlatformLog(eLogLevel ll, const char8* text)
{
	__android_log_print(LogLevelToAndtoid(ll), androidLogTag.c_str(), text);
}

void Logger::PlatformLog(eLogLevel ll, const char16* text)
{
	wprintf(L"%s", text);
}

void Logger::SetTag(const char8 *logTag)
{
	androidLogTag = Format("%s", logTag);
}


}

#endif //#if defined(__DAVAENGINE_ANDROID__)
