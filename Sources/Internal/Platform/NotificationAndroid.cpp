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



#include "NotificationAndroid.h"
#include "Platform/Notification.h"

#if defined(__DAVAENGINE_ANDROID__)

#include "Platform/TemplateAndroid/ExternC/AndroidLayer.h"

namespace DAVA
{

jclass JniNotification::gJavaClass = NULL;
const char* JniNotification::gJavaClassName = NULL;

jclass JniNotification::GetJavaClass() const
{
	return gJavaClass;
}

const char* JniNotification::GetJavaClassName() const
{
	return gJavaClassName;
}

void JniNotification::ShowNotifitaionWithProgress(uint32 id,
			const String& title,
			const String& text,
			int32 maxValue,
			int32 value)
{
	jmethodID mid = GetMethodID("NotifyProgress", "(ILjava/lang/String;Ljava/lang/String;II)V");
	if (mid)
	{
		WideString wsText(text.begin(), text.end());
		WideString wsTitle(title.begin(), title.end());

		jstring jStrTitle = CreateJString(GetEnvironment(), wsTitle);
	  	jstring jStrText = CreateJString(GetEnvironment(), wsText);
		GetEnvironment()->CallStaticVoidMethod(
						GetJavaClass(),
						mid,
						id,
						jStrTitle,
						jStrText,
						maxValue,
						value);
		GetEnvironment()->DeleteLocalRef(jStrTitle);
		GetEnvironment()->DeleteLocalRef(jStrText);
	}
}

void Notification::Hide(uint32 id)
{
	jmethodID mid = GetMethodID("HideNotification", "(I)V");
	if (mid)
	{
		GetEnvironment()->CallStaticVoidMethod(
						GetJavaClass(),
						mid,
						id);
	}
}

void Notification::SetTitle(const String &title)
{
	jmethodID mid = GetMethodID("SetNotificationTitle", "(ILjava/lang/String;)V");
	if (mid)
	{
		WideString wsTitle(title.begin(), title.end());
	  	jstring jStrTitle = CreateJString(GetEnvironment(), wsTitle);

		GetEnvironment()->CallStaticVoidMethod(
						GetJavaClass(),
						mid,
						id,
						jStrTitle);

		GetEnvironment()->DeleteLocalRef(jStrTitle);
	}
}

void Notification::SetText(const String &text)
{
	jmethodID mid = GetMethodID("SetNotificationText", "(ILjava/lang/String;)V");
	if (mid)
	{
		WideString wsText(text.begin(), text.end());
	  	jstring jStrText = CreateJString(GetEnvironment(), wsText);

		GetEnvironment()->CallStaticVoidMethod(
						GetJavaClass(),
						mid,
						id,
						jStrText);

		GetEnvironment()->DeleteLocalRef(jStrText);
	}
}

NotificationProgress::NotificationProgress()
	: Notification()
	, total(0)
	, progress(0)
{
	id = DOWNLOAD_PROGRESS;
}

void NotificationProgress::SetProgressCurrent(uint32 _currentProgress)
{
	jmethodID mid = GetMethodID("SetNotificationProgress", "(III)V");
	if (mid)
	{
		GetEnvironment()->CallStaticVoidMethod(
						GetJavaClass(),
						mid,
						id,
						total,
						_currentProgress);
		progress = _currentProgress;
	}
}

void NotificationProgress::SetProgressTotal(uint32 _total)
{
	jmethodID mid = GetMethodID("SetNotificationProgress", "(III)V");
	if (mid)
	{
		GetEnvironment()->CallStaticVoidMethod(
						GetJavaClass(),
						mid,
						id,
						_total,
						progress);
		total = _total;
	}
}

NotificationProgress *NotificationController::CreateNotificationProgress(const String &title, const String &text, uint32 maximum, uint32 current)
{
	NotificationProgress *note = new NotificationProgress();

	note->ShowNotifitaionWithProgress(Notification::DOWNLOAD_PROGRESS,
			title,
			text,
			current,
			maximum);

	notificationsList.push_back(note);

	return note;
}

}

#endif
