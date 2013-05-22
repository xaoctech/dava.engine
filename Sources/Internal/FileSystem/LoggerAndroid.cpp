/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

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

void Logger::Logv(eLogLevel ll, const char8* text, va_list li)
{
	if (ll < logLevel)return; 

	char tmp[4096];
	vsprintf(tmp, text, li);
	strcat(tmp, "\n");
	__android_log_print(LogLevelToAndtoid(ll), androidLogTag.c_str(), tmp);
}

void Logger::Logv(eLogLevel ll, const char16* text, va_list li)
{
	//TODO: VK: add code

	if (ll < logLevel)return; 

 	wchar_t tmp[4096];
	vswprintf(tmp, 4096, text, li);
 	wcscat(tmp, L"\n");

// 	char tmpChar[4096];
// 	wcstombs(tmpChar, tmp, 4096);
// 	__android_log_print(LogLevelToAndtoid(ll), LOG_TAG, tmpChar);
}

void Logger::SetTag(const char8 *logTag)
{
    androidLogTag = Format("%s", logTag);
}
    

}

#endif //#if defined(__DAVAENGINE_ANDROID__)
