#include "JniExtensions.h"
#include "Platform/TemplateAndroid/CorePlatformAndroid.h"

using namespace DAVA;

JniExtension::JniExtension(const char* className)
{
	Logger::Debug("JniExtension::JniExtension(%s)", className);
	this->className = className;
	javaClass = 0;
	isThreadAttached = false;

	CorePlatformAndroid *core = (CorePlatformAndroid *)Core::Instance();
	AndroidSystemDelegate* delegate = core->GetAndroidSystemDelegate();
	vm = delegate->GetVM();

	jint res = JNI_OK;
	res = vm->GetEnv((void **)&env,JNI_VERSION_1_6);

	/*if (res == JNI_EDETACHED)
	{
		res = vm->AttachCurrentThread(&env, NULL);
		if (res == JNI_OK)
			isThreadAttached = true;
	}*/

	if (res == JNI_OK)
	{
		javaClass = env->FindClass(className);
		if (!javaClass)
			Logger::Error("Error find class %s", className);
	}
	else
	{
		Logger::Error("Failed to get the environment using GetEnv()");
	}
}

JniExtension::~JniExtension()
{
	Logger::Debug("JniExtension::~JniExtension(%s)", className);

    ReleaseJavaClass();
    
	if (isThreadAttached)
	{
		jint res = vm->DetachCurrentThread();
		Logger::Error("vm->DetachCurrentThread() = %d", res);
		Logger::Error("Failed to DetachCurrentThread()");
	}
}

jmethodID JniExtension::GetMethodID(const char *methodName, const char *paramCode)
{
	jmethodID mid = NULL;
    
    javaClass = env->FindClass(className);
	if (javaClass)
		mid = env->GetStaticMethodID(javaClass, methodName, paramCode);

	if (!mid)
	{
		Logger::Error("get method id of %s.%s error ", className, methodName);
	}

	return mid;
}

Rect JniExtension::V2P(const Rect& srcRect) const
{
	Vector2 offset = Core::Instance()->GetPhysicalDrawOffset();
	float32 v2p = Core::Instance()->GetVirtualToPhysicalFactor();
	Rect rect = srcRect;
	rect.x *= v2p;
	rect.y *= v2p;
	rect.dx *= v2p;
	rect.dy *= v2p;

	rect += offset;
	return rect;
}


void JniExtension::ReleaseJavaClass()
{
	if (javaClass)
	{
		env->DeleteLocalRef(javaClass);
		javaClass = 0;
	}
}

