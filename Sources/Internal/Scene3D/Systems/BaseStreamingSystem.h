#pragma once

#include "Asset/Asset.h"
#include "Asset/AssetListener.h"
#include "Entity/SceneSystem.h"
#include "Math/Vector.h"
#include "Scene3D/Level.h"

#define STREAMING_DEBUG_DRAW

namespace DAVA
{
class FilePath;
class BaseStreamingSystem : public SceneSystem
{
public:
    BaseStreamingSystem(Scene* scene);
    BaseStreamingSystem(Scene* scene, const ComponentMask& requiredComponents);
    virtual ~BaseStreamingSystem();

    void Process(float32 timeElapsed) override;
    void PrepareForRemove() override;

protected:
    const Asset<Level>& GetLoadedLevel() const;
    void LoadLevelImpl(const FilePath& path);
    void UnloadLevel();

    virtual void ChunkBecomeVisible(const Level::ChunkCoord& cord) = 0;
    virtual void ChunkBecomeInvisible(const Level::ChunkCoord& cord) = 0;

private:
    void EntityOnLoaded(const Asset<AssetBase>& asset);
    void EntityOnUnloaded(const AssetBase* asset);

    Asset<Level> level;
    Vector3 cameraPosition;
    Level::ChunkCoord previousCameraPos;

    SimpleAssetListener entityAssetListener;

    Set<Level::ChunkCoord> loadedChunks;
#if defined(STREAMING_DEBUG_DRAW)
    Level::ChunkCoord currentChunk;
#endif
};
} // namespace DAVA
