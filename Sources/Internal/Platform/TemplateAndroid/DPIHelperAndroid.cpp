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
	jmethodID mid = GetMethodID("GetScreenDPI", "()I");
	if (mid)
	{
		return GetEnvironment()->CallStaticIntMethod(javaClass, mid);
	}
	return 0;
}

uint32 DPIHelper::GetScreenDPI()
{
	JniDpiHelper helper;
	return helper.GetScreenDPI();
}
