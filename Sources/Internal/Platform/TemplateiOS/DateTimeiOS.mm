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

#ifdef __DAVAENGINE_IPHONE__
/*
#import <UIKit/UIDevice.h>
#import <Foundation/NSLocale.h>
#import <sys/utsname.h>
#import <AdSupport/ASIdentifierManager.h>
*/
#import <time.h>
#import <xlocale.h>

namespace DAVA
{
    DAVA::WideString DateTime::AsWString(const wchar_t* format)
    {
        //
        DAVA::String locID = [[[NSLocale currentLocale] objectForKey: NSLocaleIdentifier] UTF8String];
        
        //
        
        struct tm timeinfo;
        wchar_t buffer [80];
        
        Timestamp timeWithTZ = innerTime + timeZoneOffset;
        timeinfo = *std::gmtime(&timeWithTZ);
        
        locale_t loc = newlocale ( LC_ALL_MASK , locID.c_str() , NULL );
        wcsftime_l (buffer,80, format, &timeinfo, loc);
        
        DAVA::WideString str(buffer);
        //
        NSString * wstr = [[NSString alloc] initWithBytes:buffer length:wcslen(buffer)*sizeof(*buffer) encoding:NSUTF32LittleEndianStringEncoding];
        NSLog(@"***AsWString : %@", wstr);
        //
        return str;
    }
}

#endif