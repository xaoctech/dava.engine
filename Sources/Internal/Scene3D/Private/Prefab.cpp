#include "Scene3D/Prefab.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Entity.h"

#include "FileSystem/FilePath.h"
#include "Logger/Logger.h"

namespace DAVA
{
Prefab::~Prefab()
{
}

void Prefab::Save(const FilePath& filepath)
{
    size_t realCnt = 0;
    size_t size = scene->GetChildrenCount();
    if (size != 1)
    {
        for (uint32 k = 0; k < size; ++k)
        {
            if (scene->GetChild(k)->GetName().find("editor.") == DAVA::String::npos)
            {
                realCnt++;
            }
        }
    }
    if (realCnt != 1)
    {
        Logger::Error("Can't save %d as prefab", filepath.GetAbsolutePathname().c_str());
    }
}
};
