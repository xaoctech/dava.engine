#ifdef __DAVAENGINE_BEAST__

#include "BeastDebug.h"

DAVA::String ConvertBeastString(ILBStringHandle h)
{
	int32 len;
	ILBGetLength(h, &len);
	DAVA::String result(len - 1, 0);
	ILBCopy(h, &result[0], len);
	ILBReleaseString(h);
	return result;
}


#endif //__DAVAENGINE_BEAST__