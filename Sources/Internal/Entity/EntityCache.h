#pragma once

#include "Base/BaseTypes.h"
#include "FileSystem/FileSystem.h"

namespace DAVA
{
class Entity;

class EntityCache
{
public:
    ~EntityCache();

    void Preload(const FilePath& path);
    void Clear(const FilePath& path);
    void ClearAll();

    Entity* GetOriginal(const FilePath& path);
    Entity* GetClone(const FilePath& path);

protected:
    Map<FilePath, Entity*> cachedEntities;
};
} // namespace DAVA