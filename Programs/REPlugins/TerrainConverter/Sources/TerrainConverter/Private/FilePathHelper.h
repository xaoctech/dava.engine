#ifndef __FILEPATH_HELPER_H__
#define __FILEPATH_HELPER_H__

#include <FileSystem/FilePath.h>
#include <Utils/StringFormat.h>
#include <Utils/Utils.h>

class FilePathHelper
{
public:
    static DAVA::FilePath GetLKAPath(const DAVA::FilePath& mapPath, const DAVA::Vector<DAVA::String>& mapTags)
    {
        DAVA::FilePath path(mapPath);
        path.ReplaceDirectory(path.GetDirectory() + "/blitz/");

        if (mapTags.empty())
        {
            path.ReplaceExtension(".lka");
        }
        else
        {
            DAVA::String mapTagsPart;
            DAVA::Merge(mapTags, '.', mapTagsPart);
            const DAVA::String lkaExtension = DAVA::Format(".%s.lka", mapTagsPart.c_str());
            path.ReplaceExtension(lkaExtension);
        }

        return path;
    }

    static DAVA::FilePath GetMKMPath(const DAVA::FilePath& mapPath)
    {
        DAVA::FilePath path(mapPath);
        path.ReplaceDirectory(path.GetDirectory() + "/blitz/");
        path.ReplaceExtension(".mkm");
        return path;
    }

    static DAVA::FilePath MapEffectsPath(const DAVA::FilePath& mapPath)
    {
        DAVA::FilePath path(mapPath);
        path.ReplaceDirectory(path.GetDirectory() + "/blitz/");
        path.ReplaceFilename("map_effects.yaml");
        return path;
    }

    //with DLC we have several resources folders, so you can't just do FilePath("~res:/"),
    //but should find res folder containging your file instead
    static DAVA::FilePath GetResFolderCointainingFile(const DAVA::String& fileName)
    {
        DAVA::FilePath pathToFile("~res:/" + fileName);
        DVASSERT(pathToFile.Exists());
        DAVA::String absPathToFile = pathToFile.GetAbsolutePathname();
        DVASSERT(DAVA::String::npos != absPathToFile.find(fileName));
        DAVA::String absPathToResFolder = absPathToFile.substr(0, absPathToFile.find(fileName));
        return absPathToResFolder;
    }
};

#endif //__FILEPATH_HELPER_H__
