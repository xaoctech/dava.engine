#pragma once

#include "Asset/Asset.h"
#include "Asset/AssetBase.h"
#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Base/Observer.h"
#include "Entity/SceneSystem.h"

namespace DAVA
{
class Level;
class FilePath;
class StreamingSystem : public SceneSystem
{
public:
    StreamingSystem(Scene* scene);
    virtual ~StreamingSystem();

    enum eStatus
    {
        NO_LEVEL = 0,
        LEVEL_BASE_PART_LOADING = 1,
        LEVEL_STREAMING = 2, // base part loaded.
    };

    void RegisterEntity(Entity* entity) override;
    void UnregisterEntity(Entity* entity) override;
    void RegisterComponent(Entity* entity, Component* component) override;
    void UnregisterComponent(Entity* entity, Component* component) override;
    void PrepareForRemove() override;

    void LoadModel(const FilePath& filepath);
    void LoadLevel(const FilePath& filepath);
    void UnloadLevel();

    void Process(float32 timeElapsed) override;

    eStatus GetStatus() const;

protected:
    void LoadLevelCompleteCallback(Asset<AssetBase> asset);
    void StreamingCallback();
    eStatus status = NO_LEVEL;
    Asset<Level> level = nullptr;
};

} // ns
