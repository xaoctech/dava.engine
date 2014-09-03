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
#include "Thread/LockGuard.h"

namespace DAVA
{

LocalNotification::LocalNotification()
#if defined(__DAVAENGINE_ANDROID__)
    : JniLocalNotification()
#elif defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_WIN32__)
    : LocalNotificationNotImplemented()
#endif
	, isChanged(false)
	, isVisible(true)
	, title(L"")
	, text(L"")
{
    static uint32 lastId = 0;
    id = ++lastId;
}
    
LocalNotification::~LocalNotification()
{

}

bool LocalNotification::IsChanged() const
{
	return isChanged;
}

bool LocalNotification::IsVisible() const
{
	return isVisible;
}

void LocalNotification::SetTitle(const WideString &_title)
{
	if (_title != title)
	{
		isChanged = true;
		title = _title;
	}
}

void LocalNotification::SetText(const WideString &_text)
{
	if (_text != text)
	{
		isChanged = true;
		text = _text;
	}
}

void LocalNotification::Show()
{
	isVisible = true;
	isChanged = true;
}

void LocalNotification::Hide()
{
	isVisible = false;
	isChanged = true;
}

void LocalNotification::Update()
{
	if (false == isChanged)
		return;

	if (true == isVisible)
		NativeShow();
	else
		NativeHide();

	isChanged = false;
}

LocalNotificationProgress::LocalNotificationProgress()
    : LocalNotification()
    , total(0)
    , progress(0)
{

}
    
LocalNotificationProgress::~LocalNotificationProgress()
{
	NativeHide();
}

void LocalNotificationProgress::SetProgressCurrent(uint32 _currentProgress)
{
	if (progress != _currentProgress)
	{
		isChanged = true;
		progress = _currentProgress;
	}
}

void LocalNotificationProgress::SetProgressTotal(uint32 _total)
{
	if (total != _total)
	{
		isChanged = true;
		total = _total;
	}
}

LocalNotificationController::~LocalNotificationController()
{
	LockGuard<Mutex> guard(notificationsListMutex);
    if (!notificationsList.empty())
    {
        for (List<LocalNotification *>::iterator it = notificationsList.begin(); it != notificationsList.end();)
        {
            LocalNotification *notification = (*it);
            it = notificationsList.erase(it);
            SafeRelease(notification);
        }
    }
}

LocalNotificationProgress *LocalNotificationController::CreateNotificationProgress(const WideString &title, const WideString &text, uint32 maximum, uint32 current)
{
	LocalNotificationProgress *note = new LocalNotificationProgress();
    
    if (NULL != note)
    {
        note->SetText(text);
        note->SetTitle(title);
        note->SetProgressCurrent(current);
        note->SetProgressTotal(maximum);
        note->SetAction(Message());

        LockGuard<Mutex> guard(notificationsListMutex);
        notificationsList.push_back(note);
    }

    return note;
}

LocalNotificationText *LocalNotificationController::CreateNotificationText(const WideString &title, const WideString &text)
{
	LocalNotificationText *note = new LocalNotificationText();

    if (NULL != note)
    {
        note->SetText(text);
        note->SetTitle(title);
        note->SetAction(Message());

        LockGuard<Mutex> guard(notificationsListMutex);
        notificationsList.push_back(note);
    }

    return note;
}

bool LocalNotificationController::Remove(LocalNotification *notification)
{
	LockGuard<Mutex> guard(notificationsListMutex);
    if (!notificationsList.empty())
    {
        for (List<LocalNotification *>::iterator it = notificationsList.begin(); it != notificationsList.end();)
        {
        	if (notification == (*it))
        	{
        		it = notificationsList.erase(it);
        		SafeRelease(notification);
        		return true;
        	}
        }
    }

    return false;
}

void LocalNotificationController::Update()
{
	LockGuard<Mutex> guard(notificationsListMutex);

    if (!notificationsList.empty())
    {
        for (List<LocalNotification *>::iterator it = notificationsList.begin(); it != notificationsList.end(); ++it)
        {
            LocalNotification *notification = (*it);

            notification->Update();
        }
    }
}

LocalNotification *LocalNotificationController::GetNotificationById(uint32 id)
{
	LockGuard<Mutex> guard(notificationsListMutex);
    if (!notificationsList.empty())
    {
        for (List<LocalNotification *>::iterator it = notificationsList.begin(); it != notificationsList.end();)
        {
        	DVASSERT(NULL != (*it));
        	if (id == (*it)->GetId())
        	{
        		return (*it);
        	}
        }
    }

    return NULL;
}

void LocalNotificationController::OnNotificationPressed(uint32 id)
{
	LocalNotification *notification = GetNotificationById(id);
	if (NULL != notification)
	{
		notification->RunAction();
	}
}

}

