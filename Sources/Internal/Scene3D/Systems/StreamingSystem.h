#pragma once

#include "Asset/Asset.h"
#include "Asset/AssetListener.h"
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
    void EntityOnLoaded(const Asset<AssetBase>& asset);
    void EntityOnError(const Asset<AssetBase>& asset, bool reloaded, const String& msg);
    void EntityOnUnloaded(const AssetBase* asset);

    void RequestLoadEntity(uint32 entityIndex, uint32 chunkAddress);
    void RequestUnloadEntity(uint32 entityIndex, uint32 chunkAddress);
    void RequestGlobalChunk();
    void RequestLoadChunk(const Level::ChunkCoord& chunkCoord);
    void RequestUnloadChunk(const Level::ChunkCoord& chunkCoord);

    void StreamEntityCallback(const Asset<AssetBase>& asset, bool reload);

    Asset<Level> level = nullptr;
    Vector3 cameraPosition;
    Level::ChunkCoord previousCameraPos;

    SimpleAssetListener levelAssetListener;
    SimpleAssetListener entityAssetListener;
};

} // ns
