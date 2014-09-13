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
#include "Notification/NotificationAndroid.h"
#include "Notification/NotificationNotImplemented.h"


namespace DAVA
{
class LocalNotificationController;

class LocalNotification
		: public BaseObject
{
#if defined(__DAVAENGINE_ANDROID__)
    typedef LocalNotificationAndroid LocalNotificationImplementation;
#else
    typedef LocalNotificationAndroid LocalNotificationImplementation;
#endif

	friend class LocalNotificationController;
protected:
	LocalNotification();
    virtual ~LocalNotification();

public:
    void SetAction(const Message& msg);
    inline void RunAction() {action(this);}

	void SetTitle(const WideString &_title);
	void SetText(const WideString &_text);
	void Show();
    void Hide();
    void Update();

    inline bool IsChanged() const {return isChanged;}
    inline bool IsVisible() const {return isVisible;}
    inline const uint32 GetId() const {return impl->id;}

private:
    virtual void ImplShow() = 0;

protected:
    LocalNotificationImplementation *impl;

    bool isChanged;
    bool isVisible;

    Message action;

    WideString title;
    WideString text;
};

class LocalNotificationProgress : public LocalNotification
{
	friend class LocalNotificationController;

protected:
	LocalNotificationProgress();
	virtual ~LocalNotificationProgress();

public:
	void SetProgressCurrent(uint32 _currentProgress);
	void SetProgressTotal(uint32 _total);

private:
	virtual void ImplShow();

private:
	uint32 total;
	uint32 progress;
};

class LocalNotificationText : public LocalNotification
{
	friend class LocalNotificationController;

private:
	virtual void ImplShow();

};

class LocalNotificationController : public Singleton<LocalNotificationController>
{
	friend class LocalNotification;
public:
    virtual ~LocalNotificationController();
	LocalNotificationProgress *CreateNotificationProgress(const WideString &title = L"", const WideString &text = L"", uint32 max = 0, uint32 current = 0);
    LocalNotificationText *CreateNotificationText(const WideString &title = L"", const WideString &text = L"");
    bool Remove(LocalNotification *notification);
    void Update();

    LocalNotification *GetNotificationById(uint32 id);
    void OnNotificationPressed(uint32 id);

private:
	Mutex notificationsListMutex;
	List<LocalNotification *> notificationsList;
};

}

#endif // __NOTIFICATION_H__
