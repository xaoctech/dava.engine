#pragma once

#include <Base/BaseTypes.h>
#include <FileSystem/FileSystem.h>

namespace DAVA
{
namespace DocumentsDirectorySetup
{
FilePath GetApplicationDocDirectory(FileSystem* fs, const String& appName);
FileSystem::eCreateDirectoryResult CreateApplicationDocDirectory(FileSystem* fs, const String& appName);
void SetApplicationDocDirectory(FileSystem* fs, const String& appName);
}
}
