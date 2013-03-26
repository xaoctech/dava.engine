#include "SceneUtils.h"

using namespace DAVA;

SceneUtils::SceneUtils()
{
    dataFolder = String("");
    dataSourceFolder = String(""); 
    workingFolder = String("");
}

SceneUtils::~SceneUtils()
{
}

void SceneUtils::CleanFolder(const String &folderPathname, Set<String> &errorLog)
{
    bool ret = FileSystem::Instance()->DeleteDirectory(folderPathname);
    if(!ret)
    {
        bool folderExists = FileSystem::Instance()->IsDirectory(folderPathname);
        if(folderExists)
        {
            errorLog.insert(String(Format("[CleanFolder] ret = %d, folder = %s", ret, folderPathname.c_str())));
        }
    }
}

void SceneUtils::SetInFolder(const String &folderPathname)
{
    dataSourceFolder = NormalizeFolderPath(folderPathname);
}

void SceneUtils::SetOutFolder(const String &folderPathname)
{
    dataFolder = NormalizeFolderPath(folderPathname);
}


String SceneUtils::NormalizeFolderPath(const String &pathname)
{
    String normalizedPathname = FileSystem::Instance()->GetCanonicalPath(pathname);

    int32 lastPos = normalizedPathname.length() - 1;
    if((0 <= lastPos) && ('/' != normalizedPathname.at(lastPos)))
    {
        normalizedPathname += "/";
    }
    
    return normalizedPathname;
}


bool SceneUtils::CopyFile(const String &filePathname, Set<String> &errorLog)
{
    String workingPathname = RemoveFolderFromPath(filePathname, dataSourceFolder);
    PrepareFolderForCopy(workingPathname, errorLog);
    
    bool retCopy = FileSystem::Instance()->CopyFile(dataSourceFolder + workingPathname, dataFolder + workingPathname);
    if(!retCopy)
    {
        errorLog.insert(String(Format("Can't copy %s from %s to %s", workingPathname.c_str(), dataSourceFolder.c_str(), dataFolder.c_str())));
    }
    
    return retCopy;
}

void SceneUtils::PrepareFolderForCopy(const String &filePathname, Set<String> &errorLog)
{
    String folder, file;
    FileSystem::SplitPath(filePathname, folder, file);
    
    String newFolderPath = dataFolder + folder;
    if(!FileSystem::Instance()->IsDirectory(newFolderPath))
    {
        FileSystem::eCreateDirectoryResult retCreate = FileSystem::Instance()->CreateDirectory(newFolderPath, true);
        if(FileSystem::DIRECTORY_CANT_CREATE == retCreate)
        {
            errorLog.insert(String(Format("Can't create folder %s", newFolderPath.c_str())));
        }
    }
    
    FileSystem::Instance()->DeleteFile(dataFolder + filePathname);
}

String SceneUtils::RemoveFolderFromPath(const String &pathname, const String &folderPathname)
{
    String workingPathname = pathname;
    String::size_type pos = workingPathname.find(folderPathname);
    if(String::npos != pos)
    {
        workingPathname = workingPathname.replace(pos, folderPathname.length(), "");
    }

    return workingPathname;
}

