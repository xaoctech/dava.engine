#ifdef __DAVAENGINE_BEAST__

#ifndef __BEAST_DEBUG__
#define __BEAST_DEBUG__

#include "Math/Matrix4.h"
#include "Debug/DVAssert.h"
#include "BeastTypes.h"
#include "BeastNames.h"

DAVA::String ConvertBeastString(DAVA_BEAST::ILBStringHandle h);
DAVA_BEAST::ILBMatrix4x4 ConvertDavaMatrix(const DAVA::Matrix4& davaMatrix);
DAVA::Matrix4 ConvertBeastMatrix(const DAVA_BEAST::ILBMatrix4x4& matrix);
DAVA_BEAST::ILBMatrix4x4 ConvertDavaMatrixNoTranspose(const DAVA::Matrix4& davaMatrix);


#define BEAST_VERIFY(command) \
{ \
	if (DAVA_BEAST::ILB_ST_SUCCESS != command) \
	{ \
		DAVA_BEAST::ILBStringHandle extendedError; \
		DAVA_BEAST::ILBGetExtendErrorInformation(&extendedError); \
		DAVA::Logger::Error("%s failed\n[%d] file:%s\nBeast error:\n%s", #command, __LINE__, __FILE__, ConvertBeastString(extendedError).c_str()); \
	} \
} \

#endif //__BEAST_DEBUG__
#endif //__DAVAENGINE_BEAST__
