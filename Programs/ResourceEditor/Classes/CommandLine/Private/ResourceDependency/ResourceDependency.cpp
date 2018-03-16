#include "Classes/CommandLine/Private/ResourceDependency/ResourceDependency.h"
#include "Classes/CommandLine/Private/ResourceDependency/SceneDependency.h"
#include "Classes/CommandLine/Private/ResourceDependency/TextureDependency.h"

#include <Logger/Logger.h>

bool ResourceDependency::GetDependencies(const DAVA::Vector<DAVA::FilePath>& resourcePathes, DAVA::Map<DAVA::FilePath, DAVA::Set<DAVA::FilePath>>& dependencyMap, DAVA::int32 requestedType)
{
    using namespace DAVA;

    for (const FilePath& path : resourcePathes)
    {
        bool result = false;
        if (path.IsEqualToExtension(".tex"))
        {
            result = TextureDependency::GetDependencies(path, dependencyMap[path], requestedType);
        }
        else if (path.IsEqualToExtension(".sc2"))
        {
            result = SceneDependency::GetDependencies(path, dependencyMap[path], requestedType);
        }

        if (result == false)
        {
            Logger::Error("Cannot retrieve dependencies from %s", path.GetStringValue().c_str());
            return false;
        }
    }

    return true;
};
