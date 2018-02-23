#pragma once

#include "Asset/AssetBase.h"
#include "Scene3D/SceneFile/SerializationContext.h"

namespace DAVA
{
class Scene;
class Prefab : public AssetBase
{
public:
    Prefab();
    ~Prefab() override;

    void Load(const FilePath& filepath) override;
    void Save(const FilePath& filepath) override;
    /*
        STREAMING_COMPLETE ? How reload can force update of model in scene if it's already added ?
     */
    void Reload() override;

    /**
        Clone prefab entities
     */
    Vector<Entity*> GetPrefabEntities() const;

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

    struct Header
    {
        enum
        {
            CURRENT_VERSION = 1,
        };

        char8 signature[4] = { 'P', 'R', 'F', 'B' };
        uint32 version = CURRENT_VERSION;
    };

private:
    Scene* scene = nullptr;
    /* Do not give direct access to this entity. This is placeholder of other entities for convinience. */
    Entity* rootEntity = nullptr;

    friend class PrefabComponent;
};
}
