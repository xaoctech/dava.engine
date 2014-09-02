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



#ifndef __NOTIFICATION_H__
#define __NOTIFICATION_H__

#include "Base/BaseTypes.h"
#include "Base/Singleton.h"
#include "Base/Message.h"
#include "Platform/NotificationAndroid.h"
#include "Platform/NotificationNotImplemented.h"


namespace DAVA
{

class LocalNotification
		: public BaseObject
#if defined(__DAVAENGINE_ANDROID__)
		, public JniLocalNotification
#elif defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_WIN32__)
        , public LocalNotificationNotImplemented
#endif
{
public:
	LocalNotification();
    virtual ~LocalNotification();

    bool IsChanged() const;
    bool IsVisible() const;

    void SetAction(const Message& msg);
    void RunAction() {action(this);}

	void SetTitle(const WideString &_title);
	void SetText(const WideString &_text);
	void Show();
    void Hide();
    inline uint32 GetId() {return id;}
    void Update();

protected:
    virtual void NativeShow() = 0;
    virtual void NativeHide();

protected:
    bool isChanged;
    bool isVisible;

    Message action;

    uint32 id;
    WideString title;
    WideString text;
};

class LocalNotificationProgress : public LocalNotification
{
public:
	LocalNotificationProgress();
	virtual ~LocalNotificationProgress();

	void SetProgressCurrent(uint32 _currentProgress);
	void SetProgressTotal(uint32 _total);

protected:
	virtual void NativeShow();

private:
	uint32 total;
	uint32 progress;
};

class LocalNotificationText : public LocalNotification
{
protected:
	virtual void NativeShow();
};

class LocalNotificationController : public Singleton<LocalNotificationController>
{
	friend class LocalNotification;
public:
    virtual ~LocalNotificationController();
	LocalNotificationProgress *CreateNotificationProgress(const WideString &title = L"", const WideString &text = L"", uint32 max = 0, uint32 current = 0);
    LocalNotificationText *CreateNotificationText(const WideString &title = L"", const WideString &text = L"");
    void Update();

    LocalNotification *GetNotificationById(uint32 id);
    void OnNotificationPressed(uint32 id);

private:
    bool Remove(LocalNotification *notification);

private:
	Mutex notificationsListMutex;
	List<LocalNotification *> notificationsList;
};

}

#endif // __NOTIFICATION_H__
