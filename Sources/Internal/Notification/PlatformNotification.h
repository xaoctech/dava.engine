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
	friend class LocalNotificationController;
protected:
	LocalNotification();
    virtual ~LocalNotification();

public:
    bool IsChanged() const;
    bool IsVisible() const;

    void SetAction(const Message& msg);
    inline void RunAction() {action(this);}

	void SetTitle(const WideString &_title);
	void SetText(const WideString &_text);
	void Show();
    void Hide();
    inline uint32 GetId() {return id;}
    void Update();

	void SetProgressCurrent(uint32 _currentProgress);
	void SetProgressTotal(uint32 _total);

protected:
    virtual void NativeShow() = 0;
    virtual void NativeHide();

protected:
#if defined(__DAVAENGINE_ANDROID__)
    JniLocalNotification *impl;
#elif
    NotificationNotImplemented *impl;
#endif;

    bool isChanged;
    bool isVisible;

    Message action;

    uint32 id;
    WideString title;
    WideString text;
};


class LocalNotificationText : public LocalNotification
{
	friend class LocalNotificationController;

protected:
	virtual void NativeShow();
};


}

#endif // __NOTIFICATION_H__
