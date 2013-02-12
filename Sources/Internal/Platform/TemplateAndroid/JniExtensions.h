#ifndef __JNI_EXTENSIONS_H__
#define __JNI_EXTENSIONS_H__

#include <jni.h>

namespace DAVA
{

class JniExtension
{
public:
	JniExtension(const char* className);

	jmethodID GetMethodID(const char *methodName, const char *paramCode);

protected:
	JNIEnv* GetEnvironment() {return env;};

protected:
	const char* className;
	jclass javaClass;
	JNIEnv* env;
	JavaVM* vm;
};

}

#endif// __JNI_EXTENSIONS_H__
