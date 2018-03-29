#pragma once

#include "Asset/Asset.h"
#include "Base/Any.h"
#include "Scene3D/SceneFile/SerializationContext.h"
#include "Reflection/Reflection.h"

namespace DAVA
{
class Scene;
class Prefab : public AssetBase
{
public:
    struct PathKey
    {
        PathKey() = default;
        PathKey(const FilePath& filepath)
            : path(filepath)
        {
        }
        FilePath path;
    };

    Prefab(const Any& assetKey);
    ~Prefab() override;

    /**
        Clone prefab entities
     */
    Vector<Entity*> ClonePrefabEntities() const;

    /**
        Construct from vector of entities
     */
    void ConstructFrom(const Vector<Entity*>& prefabEntities);

    /**
        Construct from one
     */
    void ConstructFrom(Entity* entity);

    /**
        Construct from scene
     */
    void ConstructFrom(Scene* scene);

private:
    Scene* scene = nullptr;
    /* Do not give direct access to this entity. This is placeholder of other entities for convinience. */
    Entity* rootEntity = nullptr;

    friend class PrefabComponent;
    friend class PrefabAssetLoader;

    DAVA_VIRTUAL_REFLECTION(Prefab, AssetBase);
};

template <>
bool AnyCompare<Prefab::PathKey>::IsEqual(const DAVA::Any& v1, const DAVA::Any& v2);
extern template struct AnyCompare<Prefab::PathKey>;
}
