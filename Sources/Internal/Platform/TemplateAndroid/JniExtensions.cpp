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
