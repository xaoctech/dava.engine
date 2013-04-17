#ifndef __JNI_EXTENSIONS_H__
#define __JNI_EXTENSIONS_H__

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

#endif// __JNI_EXTENSIONS_H__
