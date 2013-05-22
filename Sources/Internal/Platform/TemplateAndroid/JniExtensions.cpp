/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "JniExtensions.h"
#include "Platform/TemplateAndroid/CorePlatformAndroid.h"

namespace DAVA
{

JniExtension::JniExtension(const char* className)
{
	Logger::Debug("JniExtension::JniExtension(%s)", className);
	this->className = className;
	//isThreadAttached = false;

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

	if (res != JNI_OK)
	{
		Logger::Error("Failed to get the environment using GetEnv()");
		env = NULL;
	}
}

JniExtension::~JniExtension()
{
	Logger::Debug("JniExtension::~JniExtension(%s)", className);

	/*if (isThreadAttached)
	{
		jint res = vm->DetachCurrentThread();
		Logger::Error("vm->DetachCurrentThread() = %d", res);
		Logger::Error("Failed to DetachCurrentThread()");
	}*/
}

jclass JniExtension::GetJavaClass() const
{
	DVASSERT(env);
	if (!env)
		return NULL;

	jclass javaClass = env->FindClass(className);
	if (!javaClass)
		Logger::Error("Error find class %s", className);

	return javaClass;
}

void JniExtension::ReleaseJavaClass(jclass javaClass) const
{
	DVASSERT(env);
	if (!env)
		return;

	env->DeleteLocalRef(javaClass);
	javaClass = NULL;
}

jmethodID JniExtension::GetMethodID(jclass javaClass, const char *methodName, const char *paramCode) const
{
	DVASSERT(javaClass);
	jmethodID mid = env->GetStaticMethodID(javaClass, methodName, paramCode);

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

}//namespace DAVA
