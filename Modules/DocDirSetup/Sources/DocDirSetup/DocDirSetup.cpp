#include "DocDirSetup/DocDirSetup.h"

namespace DAVA
{
namespace DocumentsDirectorySetup
{
namespace Details
{
FilePath GetEngineDocumentsPath()
{
    return FileSystem::GetUserDocumentsPath() + "DAVAProject/";
}
} // namespace Details

/**
return ~/Library/Application Support/DAVAEngine/<appName>/
*/
FilePath GetApplicationDocDirectory(FileSystem* fs, const String& appName)
{
#if defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__)
    return Deatils::GetApplicationSupportPath() + appName + "/";
#else
    return Details::GetEngineDocumentsPath() + appName + "/";
#endif
}

FileSystem::eCreateDirectoryResult CreateApplicationDocDirectory(FileSystem* fs, const String& appName)
{
    FilePath docDirectory = GetApplicationDocDirectory(fs, appName);
    return fs->CreateDirectory(docDirectory, true);
}

void SetApplicationDocDirectory(FileSystem* fs, const String& appName)
{
    FilePath docDirectory = GetApplicationDocDirectory(fs, appName);
    fs->CreateDirectory(docDirectory, true);
    fs->SetCurrentDocumentsDirectory(docDirectory);
}
}
}
