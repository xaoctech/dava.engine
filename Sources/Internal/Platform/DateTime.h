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
    
    /* Proper ranges: */
    /* Seconds (0-59) */
    /* Minutes (0-59) */
    /* Hours (0-23)   */
    /* Day of the month (1-31), attention: 0 as param will lead to last day of prev. month */
    /* Month (0-11) */
    /* Time zone offset in seconds.For example, for U.S. Eastern Standard Time, the value is -5*60*60 */
    DateTime(uint32 year, uint32 month, uint32 day, int32 timeZoneOffset );
    DateTime(uint32 year, uint32 month, uint32 day, uint32 hour, uint32 minute, uint32 second, int32 timeZoneOffset);
    
    // return DateTime with shifted time zone offset to local one,
    // input timeStamp will be recognized as in utc
    static DateTime LocalTime(Timestamp);
    
    // return DateTime with shifted time zone offset to utc (if needed)
    static DateTime GmTime(Timestamp);
    
    static DateTime Now();
    
    DateTime ConvertToTimeZone(int32 timeZoneOffset);
    DateTime ConvertToLocalTimeZone();
    
    // Getters will return value in present time zone
    int32 GetYear() const;
    int32 GetMonth() const;
    int32 GetDay() const;
    int32 GetHour() const;
    int32 GetMinute() const;
    int32 GetSecond() const;
    
    void SetTimeZoneOffset(int32);
    inline int32 GetTimeZoneOffset() const;
    
    inline Timestamp GetTimestamp() const;
    
    // will parse string in format RFC822 or ISO8601
    bool Parse(const DAVA::String & src);
    
    // represetn data according to device locale and time zone
    DAVA::WideString AsWString(const wchar_t* format) const;
    
private:
    
    DateTime(Timestamp timeStamp, int32 timeZone);
    
    Timestamp GetRawTimeStampForCurrentTZ() const;
    
    static int32 GetLocalTimeZoneOffset();
    
    bool ParseISO8601Date(const DAVA::String&);
    
    bool ParseRFC822Date(const DAVA::String&);
    
    Timestamp   innerTime;
    int32       timeZoneOffset;// offset in seconds
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
