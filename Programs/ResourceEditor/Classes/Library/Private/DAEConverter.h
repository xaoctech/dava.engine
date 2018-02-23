#pragma once

#include "FileSystem/FilePath.h"

namespace DAEConverter
{
bool Convert(const DAVA::FilePath& path);
bool ConvertAnimations(const DAVA::FilePath& path);
}
