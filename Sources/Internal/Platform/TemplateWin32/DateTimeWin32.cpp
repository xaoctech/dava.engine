/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/
#include "Platform/DateTime.h"
#include "Utils/UTF8Utils.h"

namespace DAVA
{
    DAVA::WideString DateTime::AsWString(const wchar_t* format)
    {
		LCID Locale = GetSystemDefaultLCID();
		int nchars = GetLocaleInfoW(Locale, LOCALE_SENGLANGUAGE, NULL, 0);
		wchar_t* languageCode = new wchar_t[nchars];
		GetLocaleInfoW(Locale, LOCALE_SENGLANGUAGE, languageCode, nchars);

        DAVA::WideString locID(languageCode);
		delete languageCode;

        struct tm timeinfo;
        wchar_t buffer [80];
			
        Timestamp timeWithTZ = innerTime + timeZoneOffset;
		struct tm* convertedTime = std::gmtime(&timeWithTZ);
		DVASSERT(convertedTime);
        timeinfo = *convertedTime;
        
        _locale_t loc = _create_locale(LC_ALL, UTF8Utils::EncodeToUTF8(locID).c_str());
        _wcsftime_l(buffer,80, format, &timeinfo, loc);

        DAVA::WideString str(buffer);
//	
		Logger::Debug("%s", WStringToString(str).c_str());
//
		return str;
    }

    int32 DateTime::GetLocalTimeZoneOffset()
    {
		TIME_ZONE_INFORMATION TimeZoneInfo;
		GetTimeZoneInformation( &TimeZoneInfo );
	
		// TimeZoneInfo.Bias is the difference between local time
		// and GMT in minutes.
        return TimeZoneInfo.Bias*(-60);
    }
}