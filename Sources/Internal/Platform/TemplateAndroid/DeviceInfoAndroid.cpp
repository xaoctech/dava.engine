/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA, INC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA, INC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "../../Platform/DeviceInfo.h"
#include "../../Utils/Utils.h"

#if defined(__DAVAENGINE_ANDROID__)

#include "DeviceInfoAndroid.h"
#include "ExternC/AndroidLayer.h"

DAVA::String intermediateStr;

extern "C"
{

void Java_com_dava_framework_JNIDeviceInfo_SetJString(JNIEnv* env, jobject classthis, jstring jString)
{
	char str[256] = {0};
	CreateStringFromJni(env, jString, str);
	intermediateStr = DAVA::String(str);
}

}

namespace DAVA
{

JniDeviceInfo::JniDeviceInfo()
:	JniExtension("com/dava/framework/JNIDeviceInfo")
{
}

String JniDeviceInfo::GetVersion()
{
	jclass javaClass = GetJavaClass();
	if (!javaClass)
		return "";

	intermediateStr = "";
	jmethodID mid = GetMethodID(javaClass, "GetVersion", "()V");
	if (mid)
		GetEnvironment()->CallStaticVoidMethod(javaClass, mid, 0);

	ReleaseJavaClass(javaClass);

	return intermediateStr;
}

String JniDeviceInfo::GetManufacturer()
{
	jclass javaClass = GetJavaClass();
	if (!javaClass)
		return "";

	intermediateStr = "";
	jmethodID mid = GetMethodID(javaClass, "GetManufacturer", "()V");
	if (mid)
		GetEnvironment()->CallStaticVoidMethod(javaClass, mid, 0);

	ReleaseJavaClass(javaClass);
	return intermediateStr;
}

String JniDeviceInfo::GetModel()
{
	jclass javaClass = GetJavaClass();
	if (!javaClass)
		return "";

	intermediateStr = "";

	jmethodID mid = GetMethodID(javaClass, "GetModel", "()V");
	if (mid)
		GetEnvironment()->CallStaticVoidMethod(javaClass, mid, 0);
	ReleaseJavaClass(javaClass);

	return intermediateStr;
}

String JniDeviceInfo::GetLocale()
{
	jclass javaClass = GetJavaClass();
	if (!javaClass)
		return "";

	intermediateStr = "";

	jmethodID mid = GetMethodID(javaClass, "GetLocale", "()V");
	if (mid)
		GetEnvironment()->CallStaticVoidMethod(javaClass, mid, 0);
	ReleaseJavaClass(javaClass);

	return intermediateStr;
}

String JniDeviceInfo::GetRegion()
{
	jclass javaClass = GetJavaClass();
	if (!javaClass)
		return "";

	intermediateStr = "";
	jmethodID mid = GetMethodID(javaClass, "GetRegion", "()V");
	if (mid)
		GetEnvironment()->CallStaticVoidMethod(javaClass, mid, 0);
	ReleaseJavaClass(javaClass);

	return intermediateStr;
}

String JniDeviceInfo::GetTimeZone()
{
	jclass javaClass = GetJavaClass();
	if (!javaClass)
		return "";

	intermediateStr = "";
	jmethodID mid = GetMethodID(javaClass, "GetTimeZone", "()V");
	if (mid)
		GetEnvironment()->CallStaticVoidMethod(javaClass, mid, 0);
	ReleaseJavaClass(javaClass);

	return intermediateStr;
}

String JniDeviceInfo::GetUDID()
{
	jclass javaClass = GetJavaClass();
	if (!javaClass)
		return "";

	intermediateStr = "";
	jmethodID mid = GetMethodID(javaClass, "GetUDID", "()V");
	if (mid)
		GetEnvironment()->CallStaticVoidMethod(javaClass, mid, 0);
	ReleaseJavaClass(javaClass);

	return intermediateStr;
}

String JniDeviceInfo::GetName()
{
	jclass javaClass = GetJavaClass();
	if (!javaClass)
		return "";

	intermediateStr = "";
	jmethodID mid = GetMethodID(javaClass, "GetName", "()V");
	if (mid)
		GetEnvironment()->CallStaticVoidMethod(javaClass, mid, 0);
	ReleaseJavaClass(javaClass);

	return intermediateStr;
}

String DeviceInfo::GetVersion()
{
	JniDeviceInfo* jniDeviceInfo = new JniDeviceInfo();
	String version = jniDeviceInfo->GetVersion();
	delete jniDeviceInfo;

	return version;
}

String DeviceInfo::GetManufacturer()
{
	JniDeviceInfo* jniDeviceInfo = new JniDeviceInfo();
	String version = jniDeviceInfo->GetManufacturer();
	delete jniDeviceInfo;

	return version;
}

String DeviceInfo::GetModel()
{
	JniDeviceInfo* jniDeviceInfo = new JniDeviceInfo();
	String version = jniDeviceInfo->GetModel();
	delete jniDeviceInfo;

	return version;
}

String DeviceInfo::GetLocale()
{
	JniDeviceInfo* jniDeviceInfo = new JniDeviceInfo();
	String version = jniDeviceInfo->GetLocale();
	delete jniDeviceInfo;

	return version;
}

String DeviceInfo::GetRegion()
{
	JniDeviceInfo* jniDeviceInfo = new JniDeviceInfo();
	String version = jniDeviceInfo->GetRegion();
	delete jniDeviceInfo;

	return version;
}

String DeviceInfo::GetTimeZone()
{
	JniDeviceInfo* jniDeviceInfo = new JniDeviceInfo();
	String version = jniDeviceInfo->GetTimeZone();
	delete jniDeviceInfo;

	return version;
}

String DeviceInfo::GetUDID()
{
	JniDeviceInfo* jniDeviceInfo = new JniDeviceInfo();
	String udid = jniDeviceInfo->GetUDID();
	delete jniDeviceInfo;

	return udid;
}

WideString DeviceInfo::GetName()
{
	JniDeviceInfo* jniDeviceInfo = new JniDeviceInfo();
	String name = jniDeviceInfo->GetName();
	delete jniDeviceInfo;

	return StringToWString(name);
}

}

#endif
