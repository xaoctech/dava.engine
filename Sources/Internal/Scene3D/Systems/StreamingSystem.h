#pragma once

#include "Asset/Asset.h"
#include "Asset/AssetListener.h"
#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Base/Observer.h"
#include "Entity/SceneSystem.h"
#include "Scene3D/Level.h"
#include "Functional/Function.h"

namespace DAVA
{
class FilePath;
class StreamingSystem : public SceneSystem
{
public:
    StreamingSystem(Scene* scene);
    virtual ~StreamingSystem();

    void Process(float32 timeElapsed) override;
    void PrepareForRemove() override;

    void LoadStreamingLevel(const FilePath& filepath);

    using TLoadingProgressCallback = Function<void(uint32 /*loaded*/, uint32 /*summary count*/)>;
    void LoadFullLevel(const FilePath& filepath, const TLoadingProgressCallback& callback);
    void UnloadLevel();

protected:
    void EntityOnLoaded(const Asset<AssetBase>& asset);
    void EntityOnError(const Asset<AssetBase>& asset, bool reloaded, const String& msg);
    void EntityOnUnloaded(const AssetBase* asset);

    void RequestGlobalChunk();
    void RequestLoadChunk(const Level::ChunkCoord& chunkCoord);
    void RequestUnloadChunk(const Level::ChunkCoord& chunkCoord);
    void RequestVisibleChunk(const Level::ChunkCoord& chunkCoord);
    void RequestInvisibleChunk(const Level::ChunkCoord& chunkCoord);

    Asset<Level> level = nullptr;
    Vector3 cameraPosition;
    Level::ChunkCoord previousCameraPos;

    SimpleAssetListener entityAssetListener;

    enum eStreamingPolicy
    {
        EntityStreaming,
        VisibilityStreaming
    };
    eStreamingPolicy policy = EntityStreaming;
};

} // ns
