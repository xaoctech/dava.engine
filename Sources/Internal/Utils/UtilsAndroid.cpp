#include "Utils/Utils.h"
#include "UtilsAndroid.h"

using namespace DAVA;

#if defined(__DAVAENGINE_ANDROID__)

JniUtils::JniUtils()
    : jniUtils("com/dava/engine/Utils")
{
    disableSleepTimer = jniUtils.GetStaticMethod<void>("disableSleepTimer");
    enableSleepTimer = jniUtils.GetStaticMethod<void>("enableSleepTimer");
    openURL = jniUtils.GetStaticMethod<void, jstring>("openURL");
    generateGUID = jniUtils.GetStaticMethod<jstring>("generateGUID");
}

bool JniUtils::DisableSleepTimer()
{
    disableSleepTimer();
    return true;
}

bool JniUtils::EnableSleepTimer()
{
    enableSleepTimer();
    return true;
}

void JniUtils::OpenURL(const String& url)
{
    JNIEnv* env = JNI::GetEnv();
    jstring jUrl = env->NewStringUTF(url.c_str());
    openURL(jUrl);
    env->DeleteLocalRef(jUrl);
}

String JniUtils::GenerateGUID()
{
    JNIEnv* env = JNI::GetEnv();
    jstring jstr = generateGUID();
    DAVA::String result = JNI::ToString(jstr);
    env->DeleteLocalRef(jstr);
    return result;
}

void DAVA::DisableSleepTimer()
{
    JniUtils jniUtils;
    jniUtils.DisableSleepTimer();
}

void DAVA::EnableSleepTimer()
{
    JniUtils jniUtils;
    jniUtils.EnableSleepTimer();
}

void DAVA::OpenURL(const String& url)
{
    JniUtils jniUtils;
    jniUtils.OpenURL(url);
}

String DAVA::GenerateGUID()
{
    JniUtils jniUtils;
    return jniUtils.GenerateGUID();
}

#endif //#if defined(__DAVAENGINE_ANDROID__)
