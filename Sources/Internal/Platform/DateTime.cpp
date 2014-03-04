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

#include "DateTime.h"

namespace DAVA
{

    DateTime::DateTime(time_t UTCtime, int16 _timeZone /*= 0*/):
    time(UTCtime),
    time_zone(_timeZone)
    {
    }

    DateTime::DateTime(uint32 year, uint32 month, uint32 day, int16 _timeZone /*= 0*/):
    time_zone(_timeZone)
    {
        struct tm t = { 0 };
        t.tm_mon = month - 1;
        t.tm_year = year - 1900;
        t.tm_isdst = -1; // unknown
        t.tm_mday = 0;
        
        t.tm_hour += time_zone;
        time = mktime(&t);
    }

    DateTime::DateTime(uint32 year, uint32 month, uint32 day, uint32 hour, uint32 minute, uint32 second, int16 _timeZone /*= 0*/)
    {
        struct tm t = { 0 };
        t.tm_sec = second;
        t.tm_min = minute;
        t.tm_hour = hour;
        t.tm_mon = month - 1;
        t.tm_year = year - 1900;
        t.tm_isdst = -1; // unknown
        t.tm_mday = 0;
        
        t.tm_hour += time_zone;
        time = mktime(&t);
    }
    
    DateTime DateTime::Now()
    {
        return DateTime(std::time(NULL), GetLocalTimeZone());
    }

    DateTime DateTime::ConvertToTimeZone(int16 timeZone /*= 0*/)
    {
        return DateTime(0,0,0);
    }

    DateTime DateTime::ConvertToLocalTimeZone()
    {
        return DateTime(0,0,0);
    }

    uint32 DateTime::GetYear() const
    {
        return localtime(&time)->tm_year;
    }

    uint32 DateTime::GetMonth() const
    {
        return localtime(&time)->tm_mon;
    }

    uint32 DateTime::GetDay() const
    {
        return localtime(&time)->tm_mday;
    }

    uint32 DateTime::GetHour() const
    {
        return localtime(&time)->tm_hour;
    }

    uint32 DateTime::GetMinute() const
    {
        return localtime(&time)->tm_min;
    }

    uint32 DateTime::GetSecond() const
    {
        return localtime(&time)->tm_sec;
    }

    int16 DateTime::GetTimeZone() const
    {
        return time_zone;
    }

};