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


#ifndef __DAVAENGINE_DATE_TIME_H__
#define __DAVAENGINE_DATE_TIME_H__

#include "Base/BaseTypes.h"
#include "DAVAEngine.h"

namespace DAVA
{

typedef time_t Timestamp;
    
class DateTime
{
public:
    
    /// Proper ranges for constructos:
    /// Seconds: 0-59
    /// Minutes: 0-59
    /// Hours: 0-23
    /// Day of the month: 1-31, attention: 0 as param will lead to last day of prev. month
    /// Month: 0-11
    /// Year: 1969 and lower is not allowed
    /// Time zone offset in seconds.For example, for U.S. Eastern Standard Time, the value is -5*60*60
    DateTime(int32 year, int32 month, int32 day, int32 timeZoneOffset );
    DateTime(int32 year, int32 month, int32 day, int32 hour, int32 minute, int32 second, int32 timeZoneOffset);
    
    /// Return DateTime with shifted time zone offset to local one,
    /// input timeStamp will be recognized as in utc
    static DateTime LocalTime(Timestamp);
    
    /// Uses the value pointed by Timestamp to fill DateTime with the values that represent the corresponding time,
    /// expressed as a UTC time (i.e., the time at the GMT timezone).
    static DateTime GmTime(Timestamp);
    
    static DateTime Now();
    
    DateTime ConvertToTimeZone(int32 timeZoneOffset);
    DateTime ConvertToLocalTimeZone();
    
    /// Getters will return value in present time zone
    int32 GetYear() const;
    int32 GetMonth() const;
    int32 GetDay() const;
    int32 GetHour() const;
    int32 GetMinute() const;
    int32 GetSecond() const;
    
    void SetTimeZoneOffset(int32);
    inline int32 GetTimeZoneOffset() const;
    
    inline Timestamp GetTimestamp() const;
    
    // like "1969-07-21T02:56:15Z"
    bool ParseISO8601Date(const DAVA::String&);
    
    // like "Wed, 27 Sep 2006 21:36:45 +0200"
    bool ParseRFC822Date(const DAVA::String&);
    
    /// Represent data according to country code from localization system
    DAVA::WideString AsWString(const wchar_t* format) const;
   
private:
    
    DateTime(Timestamp timeStamp, int32 timeZone);
    
    static void GmTimeThreadSafe( tm* resultGmTime, const time_t *unixTimestamp);
    
    static void LocalTimeThreadSafe( tm* resultLocalTime, const time_t *unixTimestamp);

    static int32 GetLocalTimeZoneOffset();

    Timestamp GetRawTimeStampForCurrentTZ() const;
    
    void UpdateLocalTimeStructure();
    
    inline bool IsLeap(int32 year);
    
    inline int32 DaysFrom1970(int32 year);
    
    inline int32 DaysFrom0(int32 year);
    
    inline int32 DaysFrom1jan(int32 year, int32 month, int32 day);
    
    Timestamp InternalTimeGm(std::tm const *t);
    
    bool IsNumber(const char * s);
    
    Timestamp   innerTime;
    int32       timeZoneOffset;// offset in seconds
    tm          localTime;
};
    
int32 DateTime::GetTimeZoneOffset() const
{
    return timeZoneOffset;
}
    
Timestamp DateTime::GetTimestamp() const
{
    return innerTime;
}
};

#endif
