#include "DVAssertMessage.h"
#include "Platform/TemplateAndroid/CorePlatformAndroid.h"
#include "Platform/TemplateAndroid/JniExtensions.h"

using namespace DAVA;

class JniDVAssertMessage: public JniExtension
{
public:
	JniDVAssertMessage();
	void ShowMessage(const char* message);
};

JniDVAssertMessage::JniDVAssertMessage() :
	JniExtension("com/dava/framework/JNIAssert")
{

}

void JniDVAssertMessage::ShowMessage(const char* message)
{
	jmethodID mid = GetMethodID("Assert", "(Ljava/lang/String;)V");
	if (mid)
	{
		jstring jStrMessage = GetEnvironment()->NewStringUTF(message);
		GetEnvironment()->CallStaticVoidMethod(javaClass, mid, jStrMessage);
		GetEnvironment()->DeleteLocalRef(jStrMessage);
	}
}


void DVAssertMessage::InnerShow(const char* message)
{
	JniDVAssertMessage msg;
	msg.ShowMessage(message);
}
