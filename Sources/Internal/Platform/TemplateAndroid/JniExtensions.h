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

#ifndef __JNI_EXTENSIONS_H__
#define __JNI_EXTENSIONS_H__

#include "Base/BaseTypes.h"
#if defined(__DAVAENGINE_ANDROID__)

#include <jni.h>

namespace DAVA
{

class Rect;

class JniExtension
{
public:
	JniExtension(const char* className);
	virtual ~JniExtension();

	jclass GetJavaClass() const;
	void ReleaseJavaClass(jclass javaClass) const;
	jmethodID GetMethodID(jclass javaClass, const char *methodName, const char *paramCode) const;

protected:
	JNIEnv* GetEnvironment() {return env;};
	Rect V2P(const Rect& rect) const;

protected:
	const char* className;
	JNIEnv* env;
	JavaVM* vm;

	//bool isThreadAttached;
};

}

#endif //#if defined(__DAVAENGINE_ANDROID__)

#endif// __JNI_EXTENSIONS_H__
