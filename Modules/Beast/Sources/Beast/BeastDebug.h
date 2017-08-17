#pragma once

#include "Beast/BeastTypes.h"
#include "Beast/BeastNames.h"

#include <Base/BaseTypes.h>
#include <Math/Matrix4.h>
#include <Debug/DVAssert.h>
#include <Logger/Logger.h>

DAVA::String ConvertBeastString(ILBStringHandle h);
ILBMatrix4x4 ConvertDavaMatrix(const DAVA::Matrix4& davaMatrix);
DAVA::Matrix4 ConvertBeastMatrix(const ILBMatrix4x4& matrix);
ILBMatrix4x4 ConvertDavaMatrixNoTranspose(const DAVA::Matrix4& davaMatrix);


#define BEAST_VERIFY(command) \
{ \
	if (ILB_ST_SUCCESS != command) \
	{ \
		ILBStringHandle extendedError; \
		ILBGetExtendErrorInformation(&extendedError); \
		DAVA::Logger::Error("%s failed\n[%d] file:%s\nBeast error:\n%s", #command, __LINE__, __FILE__, ConvertBeastString(extendedError).c_str()); \
	} \
} \

