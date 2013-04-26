#include "Platform/DPIHelper.h"
#include "JniExtensions.h"

using namespace DAVA;

class JniDpiHelper: public JniExtension
{
public:
	JniDpiHelper();

	uint32 GetScreenDPI();
};

JniDpiHelper::JniDpiHelper() :
	JniExtension("com/dava/framework/JNIDpiHelper")
{

}

uint32 JniDpiHelper::GetScreenDPI()
{
	jclass javaClass = GetJavaClass();
	if (!javaClass)
		return 0;

	uint32 dpi = 0;
	jmethodID mid = GetMethodID(javaClass, "GetScreenDPI", "()I");
	if (mid)
	{
		dpi = GetEnvironment()->CallStaticIntMethod(javaClass, mid);
	}
	ReleaseJavaClass(javaClass);
	return dpi;
}

uint32 DPIHelper::GetScreenDPI()
{
	JniDpiHelper helper;
	return helper.GetScreenDPI();
}
