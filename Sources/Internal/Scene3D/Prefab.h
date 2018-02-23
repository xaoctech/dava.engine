#pragma once

#include "Asset/AssetBase.h"

namespace DAVA
{
class Scene;
class Entity;
class FilePath;

class Prefab : public AssetBase
{
public:
    Prefab() = default;
    ~Prefab() override;

    void Load(const FilePath& filepath) override;
    void Save(const FilePath& filepath) override;
    /*
        STREAMING_COMPLETE ? How reload can force update of model in scene if it's already added ?
     */
    virtual void Reload(const FilePath& filepath);

    /**
        References is just a link to original asset. And they are treated as link to original asset.
        After serialization for references it will be only Entity + PrefabComponent. 
     */
    void PlaceToScene(Scene* scene, bool reference = true);

    Scene* scene = nullptr;
    Entity* rootEntity = nullptr;
};
}
