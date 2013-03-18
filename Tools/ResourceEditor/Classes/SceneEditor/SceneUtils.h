#ifndef __SCENE_UTILS_H__
#define __SCENE_UTILS_H__

#include "DAVAEngine.h"

class SceneUtils
{
public:

	SceneUtils();
    ~SceneUtils();
    
    void CleanFolder(const DAVA::String &folderPathname, DAVA::Set<DAVA::String> &errorLog);
    
    void SetInFolder(const DAVA::String &folderPathname);
    void SetOutFolder(const DAVA::String &folderPathname);
    
    bool CopyFile(const DAVA::String &filePathname, DAVA::Set<DAVA::String> &errorLog);
    
    DAVA::String NormalizeFolderPath(const DAVA::String &pathname);
    DAVA::String RemoveFolderFromPath(const DAVA::String &pathname, const DAVA::String &folderPathname);

    void PrepareFolderForCopy(const DAVA::String &filePathname, DAVA::Set<DAVA::String> &errorLog);

public:

    DAVA::String dataFolder;
    DAVA::String dataSourceFolder;
    DAVA::String workingFolder;
};



#endif // __SCENE_UTILS_H__