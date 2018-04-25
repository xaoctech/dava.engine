#include "Entity/EntityCache.h"

#include "Scene3D/Scene.h"
#include "Scene3D/Components/TransformComponent.h"

namespace DAVA
{
EntityCache::~EntityCache()
{
    ClearAll();
}

void EntityCache::Preload(const FilePath& path)
{
    Scene* scene = new Scene();
    if (SceneFileV2::ERROR_NO_ERROR == scene->LoadScene(path))
    {
        Entity* srcRootEntity = scene;

        // try to perform little optimization:
        // if scene has single node with identity transform
        // we can skip this entity and move only its children
        if (1 == srcRootEntity->GetChildrenCount())
        {
            Entity* child = srcRootEntity->GetChild(0);
            if (1 == child->GetComponentCount())
            {
                TransformComponent* tr = srcRootEntity->GetComponent<TransformComponent>();
                if (nullptr != tr && tr->GetLocalMatrix() == Matrix4::IDENTITY)
                {
                    srcRootEntity = child;
                }
            }
        }

        auto count = srcRootEntity->GetChildrenCount();

        Vector<Entity*> tempV;
        tempV.reserve(count);
        for (auto i = 0; i < count; ++i)
        {
            tempV.push_back(srcRootEntity->GetChild(i));
        }

        Entity* dstRootEntity = new Entity();
        for (auto i = 0; i < count; ++i)
        {
            dstRootEntity->AddNode(tempV[i]);
        }

        dstRootEntity->ResetID();
        dstRootEntity->SetName(scene->GetName());
        cachedEntities[path] = dstRootEntity;
    }

    SafeRelease(scene);
}

Entity* EntityCache::GetOriginal(const FilePath& path)
{
    Entity* ret = nullptr;

    if (cachedEntities.find(path) == cachedEntities.end())
    {
        Preload(path);
    }

    auto i = cachedEntities.find(path);
    if (i != cachedEntities.end())
    {
        ret = i->second;
    }

    return ret;
}

Entity* EntityCache::GetClone(const FilePath& path)
{
    Entity* ret = nullptr;

    Entity* orig = GetOriginal(path);
    if (nullptr != orig)
    {
        ret = orig->Clone();
    }

    return ret;
}

void EntityCache::Clear(const FilePath& path)
{
    auto i = cachedEntities.find(path);
    if (i != cachedEntities.end())
    {
        SafeRelease(i->second);
        cachedEntities.erase(i);
    }
}

void EntityCache::ClearAll()
{
    for (auto& i : cachedEntities)
    {
        SafeRelease(i.second);
    }
    cachedEntities.clear();
}
} // namespace DAVA