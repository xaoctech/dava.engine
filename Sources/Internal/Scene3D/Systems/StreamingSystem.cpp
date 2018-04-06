#include "Scene3D/Systems/StreamingSystem.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"
#include "Logger/Logger.h"

namespace DAVA
{
StreamingSystem::StreamingSystem(Scene* scene)
    : BaseStreamingSystem(scene)
{
}

/*
    TODO: For special streaming entity we should check metrics, and load/unload entities according to metrics.
 */
//void StreamingSystem::RequestGlobalChunk()
//{
//    Level::Chunk* chunk = &level->loadedChunkGrid->specialStreamingSettingsChunk;
//
//    uint32 index = 0;
//    for (uint32 entityIndex : chunk->entitiesIndices)
//    {
//        Asset<LevelEntity> levelEntity = GetEngineContext()->assetManager->GetAsset<LevelEntity>(LevelEntity::Key(level.get(), entityIndex), AssetManager::ASYNC);
//
//        chunk->entitiesLoaded[index++] = levelEntity;
//    }
//}

void StreamingSystem::ChunkBecomeVisible(const Level::ChunkCoord& chunkCoord)
{
    const Asset<Level>& level = GetLoadedLevel();
    uint32 chunkAddress = level->loadedChunkGrid->GetChunkAddress(chunkCoord);
    Level::Chunk* chunk = level->loadedChunkGrid->GetChunk(chunkAddress);
    DVASSERT(chunk != nullptr);

    if (chunk->state == Level::Chunk::STATE_NOT_REQUESTED)
    {
        Logger::Debug("Request chunk load: %d %d", chunkCoord.x, chunkCoord.y);
        chunk->state = Level::Chunk::STATE_REQUESTED;

        uint32 index = 0;
        for (uint32 index = 0; index < chunk->entitiesIndices.size(); ++index)
        {
            LevelEntity::Key key(level.get(), chunk->entitiesIndices[index]);
            chunk->entitiesLoaded[index] = GetEngineContext()->assetManager->GetAsset<LevelEntity>(key, AssetManager::ASYNC);
        }
    }
}

void StreamingSystem::ChunkBecomeInvisible(const Level::ChunkCoord& chunkCoord)
{
    const Asset<Level>& level = GetLoadedLevel();
    uint32 chunkAddress = level->loadedChunkGrid->GetChunkAddress(chunkCoord);
    Level::Chunk* chunk = level->loadedChunkGrid->GetChunk(chunkAddress);
    DVASSERT(chunk != nullptr);

    if (chunk->state == Level::Chunk::STATE_REQUESTED)
    {
        Logger::Debug("Request chunk unload: %d %d", chunkCoord.x, chunkCoord.y);

        for (Asset<LevelEntity>& entity : chunk->entitiesLoaded)
        {
            entity = nullptr;
        }

        chunk->state = Level::Chunk::STATE_NOT_REQUESTED;
    }
}

void StreamingSystem::LoadLevel(const FilePath& filepath)
{
    BaseStreamingSystem::LoadLevelImpl(filepath);
}
};
