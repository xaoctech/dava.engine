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

#include "Notification/LocalNotificationImpl.h"
#include <UIKit/UIApplication.h>
#include <UIKit/UILocalNotification.h>
#import "NSStringUtils.h"

namespace DAVA
{
    
struct UILocalNotificationWrapper
{
    UILocalNotification *impl = NULL;
};

LocalNotificationIOS::LocalNotificationIOS(const uint32 _id)
    : notification(NULL)
{
    notificationId = _id;
}
    
LocalNotificationIOS::~LocalNotificationIOS()
{
    if (notification)
    {
        if (notification->impl)
        {
            [notification->impl release];
        }
        delete notification;
    }
    
}

void LocalNotificationIOS::SetAction(const WideString &_action)
{
}

void LocalNotificationIOS::Hide()
{
    if (NULL != notification)
    {
        [[UIApplication sharedApplication] cancelLocalNotification:notification->impl];
    }
}

void LocalNotificationIOS::ShowText(const WideString &title, const WideString text)
{
    if (NULL == notification)
    {
        notification = new UILocalNotificationWrapper();
        notification->impl = [[UILocalNotification alloc] init];
    }
    
    notification->impl.alertBody = [NSStringUtils NSStringFromWideString:text];
    
    NSNumber *idNum = [NSNumber numberWithInt:notificationId];

    NSArray *keys = [NSArray arrayWithObjects:@"id",@"action", nil];
    NSArray *objects = [NSArray arrayWithObjects:idNum, @"test action", nil];
    
    notification->impl.userInfo = [NSDictionary dictionaryWithObjects:objects forKeys:keys];

    [[UIApplication sharedApplication] cancelLocalNotification:notification->impl];
    [[UIApplication sharedApplication] scheduleLocalNotification:notification->impl];
}

void LocalNotificationIOS::ShowProgress(const WideString &title, const WideString text, const uint32 total, const uint32 progress)
{
}

LocalNotificationImpl *LocalNotificationImpl::Create(const uint32 _id)
{
    return new LocalNotificationIOS(_id);
}
    
}
#endif
