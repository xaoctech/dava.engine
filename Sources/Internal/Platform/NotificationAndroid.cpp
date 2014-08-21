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
#include "Platform/TemplateAndroid/CorePlatformAndroid.h"
#include "Platform/TemplateAndroid/ExternC/AndroidLayer.h"
#include "Mutex.h"

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

void Notification::Hide()
{
	javaCallMutex.Lock();

	GetEnvironment()->CallStaticVoidMethod(
					GetJavaClass(),
					GetMethodID("HideNotification", "(I)V"),
					id);
	javaCallMutex.Unlock();

	text = "";
	title = "";
}
    
void Notification::Update()
{
}

void Notification::SetTitle(const String &_title)
{
	title = _title;
}

void Notification::SetText(const String &_text)
{
	text = _text;
}

void NotificationProgress::ShowNotifitaionWithProgress(uint32 id,
			const String& title,
			const String& text,
			int32 maxValue,
			int32 value)
{
	javaCallMutex.Lock();

	JNIEnv *env = GetEnvironment();

	WideString wsText(text.begin(), text.end());
	WideString wsTitle(title.begin(), title.end());

	jstring jStrTitle = CreateJString(env, wsTitle);
	jstring jStrText = CreateJString(env, wsText);

	env->CallStaticVoidMethod(
					GetJavaClass(),
					GetMethodID("NotifyProgress", "(ILjava/lang/String;Ljava/lang/String;II)V"),
					id,
					jStrTitle,
					jStrText,
					maxValue,
					value);

	env->DeleteLocalRef(jStrTitle);
	env->DeleteLocalRef(jStrText);

	javaCallMutex.Unlock();
}

void NotificationProgress::SetProgressCurrent(uint32 _currentProgress)
{
	progress = _currentProgress;
}

void NotificationProgress::SetProgressTotal(uint32 _total)
{
	total = _total;
}

void NotificationProgress::Update()
{
	ShowNotifitaionWithProgress(id, title, text, total, progress);
}

}

#endif
