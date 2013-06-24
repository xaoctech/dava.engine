/*==================================================================================
    Copyright (c) 2008, DAVA Consulting, LLC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA Consulting, LLC nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    Revision History:
        * Created by Vitaliy Borodovsky 
=====================================================================================*/
#include "FileSystem/Logger.h"
#include "FileSystem/FileSystem.h"
#include "Debug/DVAssert.h"
#include <stdarg.h>

namespace DAVA 
{

#if defined(__DAVAENGINE_WIN32__)

#define vsnprintf _vsnprintf
#define vswprintf _vsnwprintf

#endif

void Logger::Logv(eLogLevel ll, const char8* text, va_list li)
{
	char tmp[4096] = {0};

	vsnprintf(tmp, sizeof(tmp) - 2, text, li);
	strcat(tmp, "\n");

	// always send log to custom subscribers
	CustomLog(ll, tmp);

	// print platform log or write log to file
	// only if log level is acceptable
	if (ll >= logLevel)
	{
		PlatformLog(ll, tmp);

		if(!logFilename.empty())
		{
			FileLog(ll, tmp);
		}
	}
}

void Logger::Logv(eLogLevel ll, const char16* text, va_list li)
{
	wchar_t tmp[4096] = {0};

	vswprintf(tmp, sizeof(tmp)/sizeof(wchar_t) - 2, text, li);
	wcscat(tmp, L"\n");

	// always send log to custom subscribers
	CustomLog(ll, tmp);

	// print platform log or write log to file
	// only if log level is acceptable
	if (ll >= logLevel)
	{
		PlatformLog(ll, tmp);

		if(!logFilename.empty())
		{
			FileLog(ll, tmp);
		}
	}
}

static const char8 * logLevelString[4] =
{	
	"debug",
	"info",
	"warning",
	"error" 
};
	
Logger::Logger()
{
	logLevel = LEVEL_DEBUG;
	SetLogFilename(String());
}

Logger::~Logger()
{
	for(size_t i = 0; i < customOutputs.size(); ++i)
	{
		delete customOutputs[i];
	}
}
	
Logger::eLogLevel Logger::GetLogLevel()
{
	return logLevel;
}
	
const char8 * Logger::GetLogLevelString(eLogLevel ll)
{
	return logLevelString[ll];
}
	
void Logger::SetLogLevel(eLogLevel ll)
{
	logLevel = ll;
}
	
void Logger::Log(eLogLevel ll, const char8* text, ...)
{
	if (ll < logLevel)return; 
	
	va_list vl;
	va_start(vl, text);
	Logv(ll, text, vl);
	va_end(vl);
}	

void Logger::Log(eLogLevel ll, const char16* text, ...)
{
	if (ll < logLevel) return; 
	
	va_list vl;
	va_start(vl, text);
	Logv(ll, text, vl);
	va_end(vl);
}
	
void Logger::Debug(const char8 * text, ...)
{
	va_list vl;
	va_start(vl, text);
    if (Logger::Instance())
        Logger::Instance()->Logv(LEVEL_DEBUG, text, vl);
	va_end(vl);
}
	
void Logger::Info(const char8 * text, ...)
{
	va_list vl;
	va_start(vl, text);
    if (Logger::Instance())
        Logger::Instance()->Logv(LEVEL_INFO, text, vl);
	va_end(vl);
}	
	
void Logger::Warning(const char8 * text, ...)
{
	va_list vl;
	va_start(vl, text);
    if (Logger::Instance())
        Logger::Instance()->Logv(LEVEL_WARNING, text, vl);
	va_end(vl);
}
	
void Logger::Error(const char8 * text, ...)
{
	va_list vl;
	va_start(vl, text);
    if (Logger::Instance())
        Logger::Instance()->Logv(LEVEL_ERROR, text, vl);
	va_end(vl);
}

void Logger::Debug(const char16 * text, ...)
{
	va_list vl;
	va_start(vl, text);
    if (Logger::Instance())
        Logger::Instance()->Logv(LEVEL_DEBUG, text, vl);
	va_end(vl);
}

void Logger::Info(const char16 * text, ...)
{
	va_list vl;
	va_start(vl, text);
    if (Logger::Instance())
        Logger::Instance()->Logv(LEVEL_INFO, text, vl);
	va_end(vl);
}	

void Logger::Warning(const char16 * text, ...)
{
	va_list vl;
	va_start(vl, text);
    if (Logger::Instance())
        Logger::Instance()->Logv(LEVEL_WARNING, text, vl);
	va_end(vl);
}

void Logger::Error(const char16 * text, ...)
{
	va_list vl;
	va_start(vl, text);
    if (Logger::Instance())
        Logger::Instance()->Logv(LEVEL_ERROR, text, vl);
	va_end(vl);
}

void Logger::AddCustomOutput(DAVA::LoggerOutput *lo)
{
	if(Logger::Instance() && lo)
		Logger::Instance()->customOutputs.push_back(lo);
}

void Logger::SetLogFilename(const String & filename)
{
	if(!filename.empty())
	{
		FilePath filepath = FileSystem::Instance()->GetCurrentDocumentsDirectory() + filename;
		logFilename = filepath.GetAbsolutePathname().c_str();
	}
	else
	{
		logFilename = filename;
	}
}

void Logger::FileLog(eLogLevel ll, const char8* text)
{
	if(FileSystem::Instance())
	{
		FILE * file = fopen(logFilename.c_str(), "ab");
		if(file)
		{
			fwrite(text, sizeof(char), strlen(text), file);
			fclose(file);
		}
	}
}

void Logger::FileLog(eLogLevel ll, const char16* text)
{
	if(FileSystem::Instance())
	{
		FILE * file = fopen(logFilename.c_str(), "ab");
		if(file)
		{
			fwrite(text, sizeof(wchar_t), wcslen(text), file);
			fclose(file);
		}
	}
}

void Logger::CustomLog(eLogLevel ll, const char8* text)
{
	for(size_t i = 0; i < customOutputs.size(); ++i)
	{
		customOutputs[i]->Output(ll, text);
	}
}

void Logger::CustomLog(eLogLevel ll, const char16* text)
{
	for(size_t i = 0; i < customOutputs.size(); ++i)
	{
		customOutputs[i]->Output(ll, text);
	}
}

}



