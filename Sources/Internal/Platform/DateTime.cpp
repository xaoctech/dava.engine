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

#define SKIP_WHITESPACE while (*s == ' ' || *s == '\t') s++;
#define SKIP_NON_WHITESPACE while (*s != ' ' && *s != '\t' && *s != '\0') s++;
#define CHECK_PREMATURE_END    if (*s == '\0') return false;

#define SECONDS_IN_HOUR 3600

static char *days[] = { "sun","mon","tue","wed","thu","fri","sat" };
static char *months[] = { "jan","feb","mar","apr","may","jun","jul","aug","sep","oct","nov","dec" };

inline DAVA::int32 is_leap(DAVA::int32 year)
{
    if(year % 400 == 0)
        return 1;
    if(year % 100 == 0)
        return 0;
    if(year % 4 == 0)
        return 1;
    return 0;
}
inline DAVA::int32 days_from_0(DAVA::int32 year)
{
    year--;
    return 365 * year + (year / 400) - (year/100) + (year / 4);
}
inline DAVA::int32 days_from_1970(DAVA::int32 year)
{
    static const int days_from_0_to_1970 = days_from_0(1970);
    return days_from_0(year) - days_from_0_to_1970;
}
inline DAVA::int32 days_from_1jan(DAVA::int32 year,DAVA::int32 month,DAVA::int32 day)
{
    static const DAVA::int32 days[2][12] =
    {
        { 0,31,59,90,120,151,181,212,243,273,304,334},
        { 0,31,60,91,121,152,182,213,244,274,305,335}
    };
    return days[is_leap(year)][month-1] + day - 1;
}

// calc time_t by hand to get time stamp in utc(system function
// returns it with local timezone shift only )
// code from http://stackoverflow.com/questions/16647819/timegm-cross-platform
time_t internal_timegm(std::tm const *t)
{
    int year = t->tm_year + 1900;
    int month = t->tm_mon;
    if(month > 11)
    {
        year += month/12;
        month %= 12;
    }
    else if(month < 0)
    {
        int years_diff = (-month + 11)/12;
        year -= years_diff;
        month+=12 * years_diff;
    }
    month++;
    int day = t->tm_mday;
    int day_of_year = days_from_1jan(year,month,day);
    int days_since_epoch = days_from_1970(year) + day_of_year;
    
    time_t seconds_in_day = 3600 * 24;
    time_t result = seconds_in_day * days_since_epoch + 3600 * t->tm_hour + 60 * t->tm_min + t->tm_sec;
    
    return result;
}

bool IsNumber(const char * s)
{
    for(int i = 0; i < strlen(s); ++i)
    {
        if (s[i] >= '0' && s[i] <= '9')
        {
            continue;
        }
        else
        {
            return false;
        }
    }
    
    return true;
}

namespace DAVA
{
    DateTime::DateTime(Timestamp timeStamp, int32 _timeZoneOffset):
    innerTime(timeStamp),
    timeZoneOffset(_timeZoneOffset)
    {
    }
    
    DateTime::DateTime(uint32 year, uint32 month, uint32 day, int32 _timeZoneOffset):
    timeZoneOffset(_timeZoneOffset)
    {
        struct tm t = { 0 };
        t.tm_mon = month;
        t.tm_year = year - 1900;
        t.tm_isdst = -1; // unknown
        t.tm_mday = day;
        
        innerTime = internal_timegm(&t);
        innerTime -= timeZoneOffset;// time member should be always in UTC format
        
        // dates before 1970 is not supported
        DVASSERT(innerTime>=0);
    }

    DateTime::DateTime(uint32 year, uint32 month, uint32 day, uint32 hour, uint32 minute, uint32 second, int32 _timeZoneOffset):
	timeZoneOffset(_timeZoneOffset)
    {
        struct tm t = { 0 };
        t.tm_sec = second;
        t.tm_min = minute;
        t.tm_hour = hour;
        t.tm_mon = month;
        t.tm_year = year - 1900;
        t.tm_isdst = -1; // unknown
        t.tm_mday = day;
        
        innerTime = internal_timegm(&t);
        innerTime -= timeZoneOffset;
        
        // dates before 1970 is not supported
        DVASSERT(innerTime>=0);
    }
    
    DateTime DateTime::Now()
    {
        int32 localTZOff = GetLocalTimeZoneOffset();
        return DateTime( time(NULL), localTZOff);
    }
    
    DateTime DateTime::LocalTime(Timestamp timeStamp)
    {
        return DateTime(timeStamp, GetLocalTimeZoneOffset());
    }
    
    DateTime DateTime::GmTime(Timestamp timeStamp)
    {
        int32 localTzOff = GetLocalTimeZoneOffset();
        Timestamp finalTimeStamp = timeStamp - localTzOff;
        return DateTime( finalTimeStamp, 0);
    }

    DateTime DateTime::ConvertToTimeZone(int32 _timeZoneOff)
    {;
        return DateTime(innerTime, _timeZoneOff);
    }

    DateTime DateTime::ConvertToLocalTimeZone()
    {
        return ConvertToTimeZone(GetLocalTimeZoneOffset());
    }

    int32 DateTime::GetYear() const
    {
        Timestamp timeWithTimeZone = GetRawTimeStampForCurrentTZ();
        return gmtime(&timeWithTimeZone)->tm_year + 1900;
    }

    int32 DateTime::GetMonth() const
    {
        Timestamp timeWithTimeZone = GetRawTimeStampForCurrentTZ();
        return gmtime(&timeWithTimeZone)->tm_mon;
    }

    int32 DateTime::GetDay() const
    {
        Timestamp timeWithTimeZone = GetRawTimeStampForCurrentTZ();
        return gmtime(&timeWithTimeZone)->tm_mday;
    }

    int32 DateTime::GetHour() const
    {
        Timestamp timeWithTimeZone = GetRawTimeStampForCurrentTZ();
        return gmtime(&timeWithTimeZone)->tm_hour;
    }

    int32 DateTime::GetMinute() const
    {
        Timestamp timeWithTimeZone = GetRawTimeStampForCurrentTZ();
        return gmtime(&timeWithTimeZone)->tm_min;
    }

    int32 DateTime::GetSecond() const
    {
        Timestamp timeWithTimeZone = GetRawTimeStampForCurrentTZ();
        return gmtime(&timeWithTimeZone)->tm_sec;
    }
    
    void DateTime::SetTimeZoneOffset(int32 value)
    {
        timeZoneOffset = value;
    }

    Timestamp DateTime::GetRawTimeStampForCurrentTZ() const
    {
        struct tm local_tm = *gmtime(&innerTime);
        local_tm.tm_hour += timeZoneOffset / SECONDS_IN_HOUR;
        return internal_timegm(&local_tm);
    } 

    bool DateTime::Parse(const DAVA::String & src)
    {
        bool retValue = ParseISO8601Date(src);
        if(!retValue)
        {
            retValue = ParseRFC822Date(src);
        }
        return retValue;
    }
    
    // like 1969-07-21T02:56:15Z
    bool DateTime::ParseISO8601Date(const DAVA::String& src)
    {
        // 1969-07-21T02:56:15Z
        // 1969-07-20T21:56:15-05:00
        if (src.length() < (strlen("1969-07-21T02:56:15Z")))
        {
            return false;
        }
        
		struct tm parseTime = {0};
        /// parsing date part
        {
            const std::string yr = src.substr(0, 4);
            if (!IsNumber(yr.c_str()))
            {
                return false;
            }
            
            parseTime.tm_year = atoi(yr.c_str()) - 1900;
            if (parseTime.tm_year < 0)
            {
                return false;
            }
            
            const std::string mon = src.substr(5, 2);
            if (!IsNumber(mon.c_str()))
            {
                return false;
            }
            
            parseTime.tm_mon = atoi(mon.c_str()) - 1;
            if (parseTime.tm_mon < 0 || parseTime.tm_mon > 11)
            {
                return false;
            }
            
            const std::string dy = src.substr(8, 2);
            if (!IsNumber(dy.c_str()))
            {
                return false;
            }
            
            parseTime.tm_mday = atoi(dy.c_str());
            if (parseTime.tm_mday < 1 || parseTime.tm_mday > 31)
            {
                return false;
            }
        }
        
        /// time
        {
            const std::string hr = src.substr(11, 2);
            if (!IsNumber(hr.c_str()))
            {
                return false;
            }
            
            parseTime.tm_hour = atoi(hr.c_str());
            if (parseTime.tm_hour < 0 || parseTime.tm_hour > 23)
            {
                return false;
            }
            
            const std::string mn = src.substr(14, 2);
            if (!IsNumber(mn.c_str()))
            {
                return false;
            }
            
            parseTime.tm_min = atoi(mn.c_str());
            if (parseTime.tm_min < 0 || parseTime.tm_min > 59)
            {
                return false;
            }
            
            const std::string sc = src.substr(17, 2);
            if (!IsNumber(sc.c_str()))
            {
                return false;
            }
            
            parseTime.tm_sec = atoi(sc.c_str());
            if (parseTime.tm_sec < 0 || parseTime.tm_sec > 59)
            {
                return false;
            }
            
        }
        /// time zone
        {
            int32 timeZone = 0;
            if (src[19] == 'Z')
            {
                timeZone = 0;
            }
            else
            {
                if (src[19] == '-')
                {
                    timeZone = -1;
                }
                else if (src[19] == '+')
                {
                    timeZone = 1;
                }
                else
                {
                    return false;
                }
                
                const std::string hr = src.substr(20, 2);
                if (!IsNumber(hr.c_str()))
                {
                    return false;
                }
                
                int tzHour = atoi(hr.c_str());
                if (tzHour < 0 || tzHour > 23)
                {
                    return false;
                }
                
                const std::string mn = src.substr(23, 2);
                if (!IsNumber(mn.c_str()))
                {
                    return false;
                }
                
                int tzMinute = atoi(mn.c_str());
                if (tzMinute < 0 || tzMinute > 59)
                {
                    return false;
                }
                
                timeZoneOffset = (timeZone * (tzHour * 60 + tzMinute)) * 60;
            }
        }
        innerTime = internal_timegm(&parseTime);
        return true;
    }
    
    /*
     SYNTAX
     
     date-time   =  [ day "," ] date time       ; dd mm yy
                                                ;  hh:mm:ss zzz
     
     day         =  "Mon"  / "Tue" /  "Wed"  / "Thu"
                 /  "Fri"  / "Sat" /  "Sun"
     
     date        =  1*2DIGIT month 2DIGIT       ; day month year
                                                ;  e.g. 20 Jun 82
     
     month       =  "Jan"  /  "Feb" /  "Mar"  /  "Apr"
                 /  "May"  /  "Jun" /  "Jul"  /  "Aug"
                 /  "Sep"  /  "Oct" /  "Nov"  /  "Dec"
     
     time        =  hour zone                   ; ANSI and Military
     
     hour        =  2DIGIT ":" 2DIGIT [":" 2DIGIT]
                                                ; 00:00:00 - 23:59:59
     
     zone        =  "UT"  / "GMT"               ; Universal Time
                                                ; North American : UT
                 /  "EST" / "EDT"               ;  Eastern:  - 5/ - 4
                 /  "CST" / "CDT"               ;  Central:  - 6/ - 5
                 /  "MST" / "MDT"               ;  Mountain: - 7/ - 6
                 /  "PST" / "PDT"               ;  Pacific:  - 8/ - 7
                 /  1ALPHA                      ; Military: Z = UT;
                                                ;  A:-1; (J not used)
                                                ;  M:-12; N:+1; Y:+12
                 / ( ("+" / "-") 4DIGIT )       ; Local differential
                                                ;  hours+min. (HHMM)
     
     SEMANTICS
     
     If included, day-of-week must be the day implied by the date
     specification.
     
     Time zone may be indicated in several ways.  "UT" is Univer-
     sal  Time  (formerly called "Greenwich Mean Time"); "GMT" is per-
     mitted as a reference to Universal Time.  The  military  standard
     uses  a  single  character for each zone.  "Z" is Universal Time.
     "A" indicates one hour earlier, and "M" indicates 12  hours  ear-
     lier;  "N"  is  one  hour  later, and "Y" is 12 hours later.  The
     letter "J" is not used.  The other remaining two forms are  taken
     from ANSI standard X3.51-1975.  One allows explicit indication of
     the amount of offset from UT; the other uses  common  3-character
     strings for indicating time zones in North America.
     
     */
    bool DateTime::ParseRFC822Date(const DAVA::String& src)
    {
        if (src.empty())
        {
            return false;
        }
        const char* str1 = src.c_str();
        int dayOfWeek = 0;
        
        char str[200],*s;
        s = &str[0];
        strncpy(str,str1,199);
        str[199] = '\0';
        
        // Convert to lowercase.
        int j;
        int i = 0;
        while (str[i] != '\0')
        {
            str[i] = tolower(str[i]);
            i++;
        }
        
        SKIP_WHITESPACE
        CHECK_PREMATURE_END
        
        for (j=0; j<7; j++)
        {
            if (strncmp(s,days[j],3) == 0)
            {
                break;
            }
        }
        
        if (j < 7)
        {
            SKIP_NON_WHITESPACE
            dayOfWeek = j;
            SKIP_WHITESPACE
            CHECK_PREMATURE_END
            if (*s == ',') s++;
            SKIP_WHITESPACE
            CHECK_PREMATURE_END
        }
        
        // Get the day, month, and year.
        int day;
        char monthStr[20];
        int month;
        int year;
        
        if (sscanf(s,"%d%s%d",&day,monthStr,&year) != 3) return false;
        SKIP_NON_WHITESPACE
        SKIP_WHITESPACE
        SKIP_NON_WHITESPACE
        SKIP_WHITESPACE
        SKIP_NON_WHITESPACE
        SKIP_WHITESPACE
        
        for (j=0; j<12; j++)
        {
            if (strncmp(monthStr,months[j],3) == 0)
            {
                break;
            }
        }
        if (j == 12)
        {
            return false;
        }
        month = j;
        
        if (year < 1900)
        {
            if (year < 50)
            {
                year += 2000;
            }
            else
            {
                year += 1900;
            }
        }
        
        int hour,minute,seconds;
        
        if (sscanf(s,"%d:%d:%d",&hour,&minute,&seconds) != 3)
        {
            return false;
        }
        SKIP_NON_WHITESPACE
        SKIP_WHITESPACE
        
        if (*s == '+') s++;
        char zoneStr[20];
        if (sscanf(s,"%s",zoneStr) != 1)
        {
            strcpy(zoneStr,"GMT");
        }
        
        if (zoneStr[0] == '-' ||
            (zoneStr[0] >= '0' && zoneStr[0] <= '9'))
        {
            // Do nothing.
        }
        else if (strcmp(zoneStr,"ut") == 0)
        {
            strcpy(zoneStr,"0000");
        }
        else if (strcmp(zoneStr,"gmt") == 0)
        {
            strcpy(zoneStr,"0000");
        }
        else if (strcmp(zoneStr,"est") == 0)
        {
            strcpy(zoneStr,"-0500");
        }
        else if (strcmp(zoneStr,"edt") == 0)
        {
            strcpy(zoneStr,"-0400");
        }
        else if (strcmp(zoneStr,"cst") == 0)
        {
            strcpy(zoneStr,"-0600");
        }
        else if (strcmp(zoneStr,"cdt") == 0)
        {
            strcpy(zoneStr,"-0500");
        }
        else if (strcmp(zoneStr,"mst") == 0)
        {
            strcpy(zoneStr,"-0700");
        }
        else if (strcmp(zoneStr,"mdt") == 0)
        {
            strcpy(zoneStr,"-0600");
        }
        else if (strcmp(zoneStr,"pst") == 0)
        {
            strcpy(zoneStr,"-0800");
        }
        else if (strcmp(zoneStr,"pdt") == 0)
        {
            strcpy(zoneStr,"-0700");
        }
        else if (strcmp(zoneStr,"a") == 0)
        {
            strcpy(zoneStr,"-0100");
        }
        else if (strcmp(zoneStr,"z") == 0)
        {
            strcpy(zoneStr,"0000");
        }
        else if (strcmp(zoneStr,"m") == 0)
        {
            strcpy(zoneStr,"-1200");
        }
        else if (strcmp(zoneStr,"n") == 0)
        {
            strcpy(zoneStr,"0100");
        }
        else if (strcmp(zoneStr,"y") == 0)
        {
            strcpy(zoneStr,"1200");
        }
        else
        {
            strcpy(zoneStr,"0000");
        }
        
        // Convert zoneStr from (-)hhmm to minutes.
        int hh,mm;
        int sign = 1;
        s = zoneStr;
        if (*s == '-')
        {
            sign = -1;
            s++;
        }
        if (sscanf(s,"%02d%02d",&hh,&mm) != 2)
        {
            return false;
        }
        int adjustment = sign * ((hh*60) + mm);
        struct tm t;
        t.tm_year = year - 1900;
        t.tm_mon = month;
        t.tm_wday = dayOfWeek;
        t.tm_mday = day;
        t.tm_hour = hour;
        t.tm_min = minute;
        t.tm_sec = seconds;
        innerTime = internal_timegm(&t);
        timeZoneOffset = adjustment * 60;
        //innerTime += timeZoneOffset;
        return true;
    }
};