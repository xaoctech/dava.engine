#include "JniExtensions.h"
#include "Platform/TemplateAndroid/CorePlatformAndroid.h"

using namespace DAVA;

JniExtension::JniExtension(const char* className)
{
	this->className = className;
	javaClass = 0;

	CorePlatformAndroid *core = (CorePlatformAndroid *)Core::Instance();
	AndroidSystemDelegate* delegate = core->GetAndroidSystemDelegate();
	env = delegate->GetEnvironment();
	vm = delegate->GetVM();
}

jmethodID JniExtension::GetMethodID(const char *methodName, const char *paramCode)
{
	if (vm->AttachCurrentThread(&env, 0) < 0)
	{
		Logger::Debug("Failed to AttachCurrentThread()");
		return NULL;
	}

	javaClass = env->FindClass(className);
	if (!javaClass)
	{
		Logger::Debug("Error find class %s", className);
		return NULL;
	}

	jmethodID mid = env->GetStaticMethodID(javaClass, methodName, paramCode);
	env->DeleteLocalRef(javaClass);
	if (!mid)
	{
		Logger::Debug("get method id of %s.%s error ", className, methodName);
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
