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
#include "Notification.h"

#if defined(__DAVAENGINE_ANDROID__)
#include "Platform/TemplateAndroid/CorePlatformAndroid.h"
#include "Platform/TemplateAndroid/ExternC/AndroidLayer.h"
#include "Platform/Mutex.h"
#include "Thread/LockGuard.h"

namespace DAVA
{

jclass LocalNotificationAndroid::gJavaClass = NULL;
const char* LocalNotificationAndroid::gJavaClassName = NULL;

LocalNotificationAndroid::LocalNotificationAndroid(const uint32 _id)
	: id(_id)
	, methodSetText(0)
	, methodSetProgress(0)
{
	SetJavaClass(GetEnvironment(), "com/dava/framework/JNINotificationProvider", &LocalNotificationAndroid::gJavaClass, &LocalNotificationAndroid::gJavaClassName);
}

jclass LocalNotificationAndroid::GetJavaClass() const
{
	return gJavaClass;
}

const char* LocalNotificationAndroid::GetJavaClassName() const
{
	return gJavaClassName;
}

void LocalNotificationAndroid::SetAction(const Message &msg)
{
	LockGuard<Mutex> mutexGuard(javaCallMutex);

	GetEnvironment()->CallStaticVoidMethod(
					gJavaClass,
					GetMethodID("EnableTapAction", "(I)V"),
					id);
}

void LocalNotificationAndroid::Hide()
{
	LockGuard<Mutex> mutexGuard(javaCallMutex);

	GetEnvironment()->CallStaticVoidMethod(
					gJavaClass,
					GetMethodID("HideNotification", "(I)V"),
					id);
}

void LocalNotificationAndroid::ShowText(const WideString &title, const WideString text)
{
	LockGuard<Mutex> mutexGuard(javaCallMutex);

	JNIEnv *env = GetEnvironment();

	jstring jStrTitle = CreateJString(env, title);
	jstring jStrText = CreateJString(env, text);

	static const jmethodID methodSetText = GetMethodID("NotifyText", "(ILjava/lang/String;Ljava/lang/String;)V");
	DVASSERT(0 != methodSetText);

	env->CallStaticVoidMethod(
					gJavaClass,
					methodSetText,
					id,
					jStrTitle,
					jStrText);

	env->DeleteLocalRef(jStrTitle);
	env->DeleteLocalRef(jStrText);
}


void LocalNotificationAndroid::ShowProgress(const WideString &title, const WideString text, const uint32 total, const uint32 progress)
{
	LockGuard<Mutex> mutexGuard(javaCallMutex);

	JNIEnv *env = GetEnvironment();

	jstring jStrTitle = CreateJString(env, title);
	jstring jStrText = CreateJString(env, text);

	static const jmethodID methodSetProgress = GetMethodID("NotifyProgress", "(ILjava/lang/String;Ljava/lang/String;II)V");
	DVASSERT(0 != methodSetProgress);

	env->CallStaticVoidMethod(
					gJavaClass,
					methodSetProgress,
					id,
					jStrTitle,
					jStrText,
					total,
					progress);

	env->DeleteLocalRef(jStrTitle);
	env->DeleteLocalRef(jStrText);
}

}

extern "C"
{

	void Java_com_dava_framework_JNINotificationProvider_onNotificationPressed(JNIEnv* env, jobject classthis, uint32_t id)
	{
		DAVA::LocalNotificationController::Instance()->OnNotificationPressed(id);
	}
}

#endif
