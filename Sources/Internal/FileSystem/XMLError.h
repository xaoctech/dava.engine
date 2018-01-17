#pragma once

#include "Base/BaseTypes.h"
#include "FileSystem/FilePath.h"

namespace DAVA
{
struct XMLError
{
    int32 code = 0;
    String message;
    int32 line = 0;
    int32 position = 0;
};
}