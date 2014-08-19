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



#include "Notification.h"
#include "Base/BaseTypes.h"

namespace DAVA
{

Notification::Notification()
#if defined(__DAVAENGINE_ANDROID__)
    : JniNotification()
#elif defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_WIN32__)
    : NotificationNotImplemented()
#endif
	, title("")
	, text("")
{
    static uint32 lastId = 0;
    id = ++lastId;
}
    
Notification::~Notification()
{
}
    
NotificationProgress::NotificationProgress()
    : Notification()
    , total(0)
    , progress(0)
{

}
    
NotificationController::~NotificationController()
{
    if (!notificationsList.empty())
    {
        for (List<Notification *>::iterator it = notificationsList.begin(); it != notificationsList.end();)
        {
            Notification *notification = (*it);
            SafeDelete(notification);
            it = notificationsList.erase(it);
        }
    }
}

NotificationProgress *NotificationController::CreateNotificationProgress(const String &title, const String &text, uint32 maximum, uint32 current)
{
    NotificationProgress *note = new NotificationProgress();
    
    if (NULL != note)
    {
        note->SetText(text);
        note->SetTitle(title);
        note->SetProgressCurrent(0);
        note->SetProgressTotal(0);
        
        note->CreateNative();
        
        notificationsList.push_back(note);
    }
    
    return note;
}

}

