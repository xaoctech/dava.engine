#ifdef __DAVAENGINE_BEAST__

#ifndef __BEAST_DEBUG__
#define __BEAST_DEBUG__

#include "DAVAEngine.h"
#include "BeastTypes.h"

DAVA::String ConvertBeastString(ILBStringHandle h)
{
	int32 len;
	ILBGetLength(h, &len);
	DAVA::String result(len - 1, 0);
	ILBCopy(h, &result[0], len);
	ILBReleaseString(h);
	return result;
}

#define BEAST_VERIFY(command) \
{ \
	if(ILB_ST_SUCCESS != command) \
	{ \
		ILBStringHandle extendedError; \
		ILBGetExtendErrorInformation(&extendedError); \
		DAVA::Logger::Error("%s file:%s line:%d Beast failed with error %s", #command, __FILE__, __LINE__, ConvertBeastString(extendedError)); \
	} \
} \

#endif //__BEAST_DEBUG__
#endif //__DAVAENGINE_BEAST__
