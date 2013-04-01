#ifndef __JNI_EXTENSIONS_H__
#define __JNI_EXTENSIONS_H__

#include <jni.h>

namespace DAVA
{

class JniExtension
{
public:
	JniExtension(const char* className);
	virtual ~JniExtension();

	jmethodID GetMethodID(const char *methodName, const char *paramCode);

protected:
	JNIEnv* GetEnvironment() {return env;};
	Rect V2P(const Rect& rect) const;

    void ReleaseJavaClass();
    
protected:
	const char* className;
	jclass javaClass;
	JNIEnv* env;
	JavaVM* vm;

	bool isThreadAttached;
};

}

#endif// __JNI_EXTENSIONS_H__
