#include "Scene3D/Systems/BaseStreamingSystem.h"

#include "Asset/AssetManager.h"
#include "Engine/Engine.h"
#include "Engine/EngineContext.h"
#include "Functional/Function.h"
#include "Render/Highlevel/RenderSystem.h"
#include "Render/RenderHelper.h"
#include "Scene3D/Scene.h"

namespace DAVA
{
BaseStreamingSystem::BaseStreamingSystem(Scene* scene)
    : SceneSystem(scene, 0)
{
    entityAssetListener.onLoaded = MakeFunction(this, &BaseStreamingSystem::EntityOnLoaded);
    entityAssetListener.onUnloaded = MakeFunction(this, &BaseStreamingSystem::EntityOnUnloaded);
}

BaseStreamingSystem::BaseStreamingSystem(Scene* scene, const ComponentMask& requiredComponents)
    : SceneSystem(scene, requiredComponents)
{
}

BaseStreamingSystem::~BaseStreamingSystem()
{
    DVASSERT(level == nullptr);
}

void BaseStreamingSystem::Process(float32 timeElapsed)
{
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
                        bool inserted = loadedChunks.insert(currentPos).second;

                        if (inserted == true)
                        {
                            ChunkBecomeVisible(currentPos);
                        }
                    }
                    else
                    {
                        loadedChunks.erase(currentPos);
                        ChunkBecomeInvisible(currentPos);
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
        for (int32 y = min.y; y <= max.y + 1; ++y)
        {
            for (int32 x = min.x; x <= max.x + 1; ++x)
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

void BaseStreamingSystem::PrepareForRemove()
{
    UnloadLevel();
}

void BaseStreamingSystem::LoadLevelImpl(const FilePath& path)
{
    AssetManager* assetManager = GetEngineContext()->assetManager;
    level = assetManager->GetAsset<Level>(Level::Key(path), AssetManager::SYNC);
    previousCameraPos = Level::ChunkCoord(std::numeric_limits<int32>::min(), std::numeric_limits<int32>::min());
    assetManager->RegisterListener(&entityAssetListener, Type::Instance<LevelEntity>());
}

void BaseStreamingSystem::UnloadLevel()
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

const Asset<Level>& BaseStreamingSystem::GetLoadedLevel() const
{
    return level;
}

void BaseStreamingSystem::EntityOnLoaded(const Asset<AssetBase>& asset)
{
    LevelEntity::Key streamEntityKey = asset->GetAssetKey<LevelEntity::Key>();
    if (streamEntityKey.level == level.get())
    {
        Asset<LevelEntity> levelEntity = AssetAs<LevelEntity>(asset);
        DVASSERT(levelEntity != nullptr);
        level->entitiesAddedToScene.emplace(streamEntityKey.entityIndex, levelEntity->rootEntity);
        GetScene()->AddEntity(levelEntity->rootEntity);
    }
}

void BaseStreamingSystem::EntityOnUnloaded(const AssetBase* asset)
{
    LevelEntity::Key streamEntityKey = asset->GetAssetKey<LevelEntity::Key>();
    if (streamEntityKey.level == level.get())
    {
        const LevelEntity* levelEntity = AssetAs<const LevelEntity>(asset);
        DVASSERT(levelEntity != nullptr);

        level->entitiesAddedToScene.erase(streamEntityKey.entityIndex);
        GetScene()->RemoveEntity(levelEntity->rootEntity);
    }
}

} // namespace DAVA
