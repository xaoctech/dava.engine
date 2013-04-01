#include "../../Platform/DeviceInfo.h"

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
	jmethodID mid = GetMethodID("GetVersion", "()V");
	if (mid)
	{
		GetEnvironment()->CallStaticVoidMethod(javaClass, mid, 0);

		return intermediateStr;
	}
}

String JniDeviceInfo::GetManufacturer()
{
	jmethodID mid = GetMethodID("GetManufacturer", "()V");
	if (mid)
	{
		GetEnvironment()->CallStaticVoidMethod(javaClass, mid, 0);

		return intermediateStr;
	}
}

String JniDeviceInfo::GetModel()
{
	jmethodID mid = GetMethodID("GetModel", "()V");
	if (mid)
	{
		GetEnvironment()->CallStaticVoidMethod(javaClass, mid, 0);

		return intermediateStr;
	}
}

String JniDeviceInfo::GetLocale()
{
	jmethodID mid = GetMethodID("GetLocale", "()V");
	if (mid)
	{
		GetEnvironment()->CallStaticVoidMethod(javaClass, mid, 0);

		return intermediateStr;
	}
}

String JniDeviceInfo::GetRegion()
{
	jmethodID mid = GetMethodID("GetRegion", "()V");
	if (mid)
	{
		GetEnvironment()->CallStaticVoidMethod(javaClass, mid, 0);

		return intermediateStr;
	}
}

String JniDeviceInfo::GetTimeZone()
{
	jmethodID mid = GetMethodID("GetTimeZone", "()V");
	if (mid)
	{
		GetEnvironment()->CallStaticVoidMethod(javaClass, mid, 0);

		return intermediateStr;
	}
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

}

#endif
