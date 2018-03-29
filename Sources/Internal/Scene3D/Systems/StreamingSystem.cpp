#include "Engine/Engine.h"
#include "Scene3D/Systems/StreamingSystem.h"
#include "Scene3D/Components/PrefabComponent.h"
#include "Scene3D/Entity.h"
#include "Scene3D/AssetLoaders/LevelAssetLoader.h"
#include "Scene3D/Scene.h"
#include "Render/Highlevel/RenderSystem.h"
#include "Render/RenderHelper.h"
#include "Logger/Logger.h"

namespace DAVA
{
StreamingSystem::StreamingSystem(Scene* scene)
    : SceneSystem(scene, 0)
{
    entityAssetListener.onLoaded = MakeFunction(this, &StreamingSystem::EntityOnLoaded);
    entityAssetListener.onError = MakeFunction(this, &StreamingSystem::EntityOnError);
    entityAssetListener.onUnloaded = MakeFunction(this, &StreamingSystem::EntityOnUnloaded);
}

StreamingSystem::~StreamingSystem()
{
    DVASSERT(level == nullptr);
}

void StreamingSystem::PrepareForRemove()
{
    UnloadLevel();
}

void StreamingSystem::LoadStreamingLevel(const FilePath& filepath)
{
    level = GetEngineContext()->assetManager->GetAsset<Level>(Level::Key(filepath), AssetManager::SYNC);
    previousCameraPos = Level::ChunkCoord(std::numeric_limits<int32>::min(), std::numeric_limits<int32>::min());
    GetEngineContext()->assetManager->RegisterListener(&entityAssetListener, Type::Instance<LevelEntity>());
    policy = EntityStreaming;
}

void StreamingSystem::LoadFullLevel(const FilePath& filepath, const TLoadingProgressCallback& callback)
{
    level = GetEngineContext()->assetManager->GetAsset<Level>(Level::Key(filepath), AssetManager::SYNC);
    previousCameraPos = Level::ChunkCoord(std::numeric_limits<int32>::min(), std::numeric_limits<int32>::min());
    GetEngineContext()->assetManager->RegisterListener(&entityAssetListener, Type::Instance<LevelEntity>());
    policy = VisibilityStreaming;

    auto foreachChunk = [](Level* level, const Function<void(Level::Chunk * chunk)>& fn) {
        fn(&level->loadedChunkGrid->specialStreamingSettingsChunk);

        for (Level::Chunk& chunk : level->loadedChunkGrid->chunkData)
        {
            fn(&chunk);
        }
    };

    uint32 entitiesCount = 0;
    auto calcEntitiesCount = [&entitiesCount](Level::Chunk* chunk) {
        entitiesCount += static_cast<uint32>(chunk->entitiesIndices.size());
    };

    uint32 loadedCount = 0;
    AssetManager* assetManager = GetEngineContext()->assetManager;
    auto loadEntities = [&callback, &loadedCount, entitiesCount, this, assetManager](Level::Chunk* chunk) {
        uint32 index = 0;
        for (uint32 entityIndex : chunk->entitiesIndices)
        {
            LevelEntity::Key key(level.get(), entityIndex);
            Asset<LevelEntity> levelEntity = assetManager->GetAsset<LevelEntity>(key, AssetManager::SYNC);
            chunk->entitiesLoaded[index++] = levelEntity;
            levelEntity->rootEntity->SetVisible(false);
            callback(++loadedCount, entitiesCount);
        }
    };

    foreachChunk(level.get(), calcEntitiesCount);

    callback(0, entitiesCount);
    foreachChunk(level.get(), loadEntities);
}

void StreamingSystem::UnloadLevel()
{
    if (level == nullptr)
    {
        return;
    }

    for (Level::Chunk& chunk : level->loadedChunkGrid->chunkData)
    {
        chunk.entitiesLoaded.clear();
    }
    GetEngineContext()->assetManager->UnregisterListener(&entityAssetListener);
    level = nullptr;
}

void StreamingSystem::EntityOnLoaded(const Asset<AssetBase>& asset)
{
    Asset<LevelEntity> levelEntity = std::dynamic_pointer_cast<LevelEntity>(asset);
    DVASSERT(levelEntity != nullptr);

    LevelEntity::Key streamEntityKey = levelEntity->GetAssetKey<LevelEntity::Key>();

    level->entitiesAddedToScene.emplace(streamEntityKey.entityIndex, levelEntity->rootEntity);
    GetScene()->AddEntity(levelEntity->rootEntity);
}

void StreamingSystem::EntityOnError(const Asset<AssetBase>&, bool reloaded, const String& msg)
{
    DVASSERT(0 && "Something strange happened");
}

void StreamingSystem::EntityOnUnloaded(const AssetBase* asset)
{
    const LevelEntity* levelEntity = dynamic_cast<const LevelEntity*>(asset);
    DVASSERT(levelEntity != nullptr);

    LevelEntity::Key streamEntityKey = levelEntity->GetAssetKey<LevelEntity::Key>();

    level->entitiesAddedToScene.erase(streamEntityKey.entityIndex);
    GetScene()->RemoveEntity(levelEntity->rootEntity);
}

/*
    TODO: For special streaming entity we should check metrics, and load/unload entities according to metrics.
 */
void StreamingSystem::RequestGlobalChunk()
{
    Level::Chunk* chunk = &level->loadedChunkGrid->specialStreamingSettingsChunk;

    uint32 index = 0;
    for (uint32 entityIndex : chunk->entitiesIndices)
    {
        Asset<LevelEntity> levelEntity = GetEngineContext()->assetManager->GetAsset<LevelEntity>(LevelEntity::Key(level.get(), entityIndex), AssetManager::ASYNC);

        chunk->entitiesLoaded[index++] = levelEntity;
    }
}

void StreamingSystem::RequestLoadChunk(const Level::ChunkCoord& chunkCoord)
{
    uint32 chunkAddress = level->loadedChunkGrid->GetChunkAddress(chunkCoord);
    Level::Chunk* chunk = level->loadedChunkGrid->GetChunk(chunkAddress);
    DVASSERT(chunk != nullptr);

    if (chunk->state == Level::Chunk::STATE_NOT_REQUESTED)
    {
        Logger::Debug("Request chunk load: %d %d", chunkCoord.x, chunkCoord.y);
        chunk->state = Level::Chunk::STATE_REQUESTED;

        uint32 index = 0;
        for (uint32 entityIndex : chunk->entitiesIndices)
        {
            Asset<LevelEntity> levelEntity = GetEngineContext()->assetManager->GetAsset<LevelEntity>(LevelEntity::Key(level.get(), entityIndex), AssetManager::ASYNC);

            chunk->entitiesLoaded[index] = levelEntity;

            index++;
        }
    }
}

void StreamingSystem::RequestUnloadChunk(const Level::ChunkCoord& chunkCoord)
{
    uint32 chunkAddress = level->loadedChunkGrid->GetChunkAddress(chunkCoord);
    Level::Chunk* chunk = level->loadedChunkGrid->GetChunk(chunkAddress);
    DVASSERT(chunk != nullptr);

    if (chunk->state == Level::Chunk::STATE_REQUESTED)
    {
        Logger::Debug("Request chunk unload: %d %d", chunkCoord.x, chunkCoord.y);

        uint32 index = 0;
        for (uint32 entityIndex : chunk->entitiesIndices)
        {
            chunk->entitiesLoaded[index] = nullptr;
            index++;
        }

        chunk->state = Level::Chunk::STATE_NOT_REQUESTED;
    }
}

void StreamingSystem::RequestVisibleChunk(const Level::ChunkCoord& chunkCoord)
{
    uint32 chunkAddress = level->loadedChunkGrid->GetChunkAddress(chunkCoord);
    Level::Chunk* chunk = level->loadedChunkGrid->GetChunk(chunkAddress);

    uint32 index = 0;
    for (uint32 entityIndex : chunk->entitiesIndices)
    {
        Asset<LevelEntity> entity = chunk->entitiesLoaded[index];
        entity->rootEntity->SetVisible(true);
    }
}

void StreamingSystem::RequestInvisibleChunk(const Level::ChunkCoord& chunkCoord)
{
    uint32 chunkAddress = level->loadedChunkGrid->GetChunkAddress(chunkCoord);
    Level::Chunk* chunk = level->loadedChunkGrid->GetChunk(chunkAddress);

    uint32 index = 0;
    for (uint32 entityIndex : chunk->entitiesIndices)
    {
        Asset<LevelEntity> entity = chunk->entitiesLoaded[index];
        entity->rootEntity->SetVisible(false);
    }
}

//#define STREAMING_DEBUG_DRAW
void StreamingSystem::Process(float32 timeElapsed)
{
#if defined(STREAMING_DEBUG_DRAW)
    static Set<Level::ChunkCoord> loadedChunks;
    static Level::ChunkCoord currentChunk;
#endif
    if (level)
    {
        Camera* camera = GetScene()->GetCurrentCamera();

        if (!camera)
            return;

        Vector3 movementDirection = camera->GetPosition() - cameraPosition;
        cameraPosition = camera->GetPosition();

        Level::ChunkCoord currentCameraPosInChunk = level->loadedChunkGrid->GetChunkCoord(cameraPosition);

        float32 visibilityRadiusMeters = 200.0f; // 1000 meters visibility
        int32 chunkVisibilityRadius = 5; //(visibilityRadiusMeters / loadedChunkGrid->chunkSize) / 2;

        if (previousCameraPos != currentCameraPosInChunk)
        {
            int32 shifts[8][2] = { { -1, 0 }, { 1, 0 }, { 0, -1 }, { 0, 1 }, { -1, -1 }, { -1, 1 }, { 1, -1 }, { 1, 1 } };

            std::queue<Level::ChunkCoord> queue;
            queue.push(currentCameraPosInChunk);
#if defined(STREAMING_DEBUG_DRAW)
            currentChunk = currentCameraPosInChunk;
#endif

            Level::ChunkGrid* loadedChunkGrid = level->loadedChunkGrid;

            while (!queue.empty())
            {
                Level::ChunkCoord currentPos = queue.front();
                queue.pop();

                if ((currentPos.x < loadedChunkGrid->worldChunkBounds.min.x)
                    || (currentPos.x > loadedChunkGrid->worldChunkBounds.max.x)
                    || (currentPos.y < loadedChunkGrid->worldChunkBounds.min.y)
                    || (currentPos.y > loadedChunkGrid->worldChunkBounds.max.y))
                {
                    // Exit from streaming update if we out of streaming area
                    // STREAMING_COMPLETE Should we unload all chunks???
                    break;
                }

                Level::ChunkCoord distance;
                distance.x = currentPos.x - currentCameraPosInChunk.x;
                distance.y = currentPos.y - currentCameraPosInChunk.y;
                int32 squareDistance = distance.x * distance.x + distance.y * distance.y;
                uint32 address = loadedChunkGrid->GetChunkAddress(currentPos);

                Level::Chunk* chunk = loadedChunkGrid->GetChunk(address);
                uint32 globalFrameIndex = Engine::Instance()->GetGlobalFrameIndex();
                if (chunk->visitedLastFrameIndex != globalFrameIndex)
                {
                    chunk->visitedLastFrameIndex = globalFrameIndex;
                    if (squareDistance < chunkVisibilityRadius * chunkVisibilityRadius)
                    {
#if defined(STREAMING_DEBUG_DRAW)
                        loadedChunks.insert(currentPos);
#endif
                        if (policy == EntityStreaming)
                            RequestLoadChunk(currentPos);
                        else
                            RequestVisibleChunk(currentPos);
                    }
                    else
                    {
#if defined(STREAMING_DEBUG_DRAW)
                        loadedChunks.erase(currentPos);
#endif
                        if (policy == EntityStreaming)
                            RequestUnloadChunk(currentPos);
                        else
                            RequestInvisibleChunk(currentPos);
                    }

                    for (uint32 direction = 0; direction < 8; ++direction)
                    {
                        Level::ChunkCoord newPos = currentPos;
                        newPos.x += shifts[direction][0];
                        newPos.y += shifts[direction][1];
                        if ((newPos.x >= loadedChunkGrid->worldChunkBounds.min.x)
                            && (newPos.x <= loadedChunkGrid->worldChunkBounds.max.x)
                            && (newPos.y >= loadedChunkGrid->worldChunkBounds.min.y)
                            && (newPos.y <= loadedChunkGrid->worldChunkBounds.max.y))
                        {
                            queue.push(newPos);
                        }
                    }
                }
            }
            previousCameraPos = currentCameraPosInChunk;
        }
        
#if defined(STREAMING_DEBUG_DRAW)
        Level::ChunkCoord min = level->loadedChunkGrid->worldChunkBounds.min;
        Level::ChunkCoord max = level->loadedChunkGrid->worldChunkBounds.max;
        RenderHelper* drawer = GetScene()->GetRenderSystem()->GetDebugDrawer();

        float32 chunkSize = level->loadedChunkGrid->chunkSize;
        float32 halfChunkSize = chunkSize * 0.5;
        for (int32 y = min.y; y <= max.y; ++y)
        {
            for (int32 x = min.x; x <= max.x; ++x)
            {
                float32 centerX = x * chunkSize + halfChunkSize;
                float32 centerY = y * chunkSize + halfChunkSize;
                AABBox3 box(Vector3(centerX, centerY, 0.0), chunkSize);

                Level::ChunkCoord coord(x, y);
                Color boxColor = Color(0.5, 0.5, 0.5, 0.3);
                if (coord == currentChunk)
                {
                    boxColor = Color(0.0, 1.0, 0.0, 0.5);
                }
                else if (loadedChunks.count(coord) > 0)
                {
                    boxColor = Color(0.0, 0.0, 1.0, 0.5);
                }

                drawer->DrawAABox(box, boxColor, RenderHelper::DRAW_SOLID_DEPTH);
            }
        }
#endif
    }
}
};
