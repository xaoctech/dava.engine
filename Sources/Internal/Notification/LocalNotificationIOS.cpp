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

#include "Notification/LocalNotificationIOS.h"

#if defined(__DAVAENGINE_IPHONE__)

#include <UIKit/UILocalNotification.h>
#include <UIKit/UIApplication.h>

namespace DAVA
{

LocalNotificationIOS::LocalNotificationIOS(const uint32 _id)
    : id(_id)
{
}
    
LocalNotificationIOS::~LocalNotificationIOS()
{
}

void LocalNotificationIOS::SetAction(const Message &msg)
{
}

void LocalNotificationIOS::Hide()
{
}
void LocalNotificationIOS::ShowText(const WideString &title, const WideString text)
{
    WideString txt = L"Test text";
    NSStringEncoding encoding = CFStringConvertEncodingToNSStringEncoding(kCFStringEncodingUTF32LE);
	NSString* nsstring = [[NSString alloc]
						  initWithBytes:(const char*)txt.c_str()
						  length:txt.length() * sizeof(wchar_t)
						  encoding:encoding];
    UILocalNotification *note = [[UILocalNotification alloc] init];
    note.alertBody = nsstring;
    
    [[UIApplication sharedApplication] scheduleLocalNotification:note];
    
}
void LocalNotificationIOS::ShowProgress(const WideString &title, const WideString text, const uint32 total, const uint32 progress)
{
}
    
}
#endif
