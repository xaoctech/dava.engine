#include "Engine/Engine.h"
#include "Scene3D/Systems/StreamingSystem.h"
#include "Scene3D/Components/PrefabComponent.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Level.h"
#include "Scene3D/AssetLoaders/LevelAssetLoader.h"
#include "Scene3D/Scene.h"

namespace DAVA
{
StreamingSystem::StreamingSystem(Scene* scene)
    : SceneSystem(scene, 0)
{
    levelAssetListener.onLoaded = MakeFunction(this, &StreamingSystem::EntityOnLoaded);
    levelAssetListener.onError = MakeFunction(this, &StreamingSystem::EntityOnError);
    levelAssetListener.onUnloaded = MakeFunction(this, &StreamingSystem::EntityOnUnloaded);
}

StreamingSystem::~StreamingSystem()
{
}

void StreamingSystem::RegisterEntity(Entity* entity)
{
    /*
    for (Component* comp : entity->GetComponents())
    {
        RegisterComponent(entity, comp);
    }
*/
}
void StreamingSystem::UnregisterEntity(Entity* entity)
{
    /*
    for (Component* comp : entity->GetComponents())
    {
        UnregisterComponent(entity, comp);
    }
 */
}

void StreamingSystem::RegisterComponent(Entity* entity, Component* component)
{
    // TODO: PrefabInstanceComponent
    if (component->GetType() == Type::Instance<PrefabComponent>())
    {
        PrefabComponent* prefabComponent = static_cast<PrefabComponent*>(component);
        if (prefabComponent->GetPrefab())
        {
            Vector<Entity*> prefabEntities = prefabComponent->GetPrefab()->GetPrefabEntities();
            for (Entity* prefabChildren : prefabEntities)
            {
                ScopedPtr<Entity> entityClone(prefabChildren->Clone());
                entity->AddEntity(entityClone);
            }
        }
    }
}

void StreamingSystem::UnregisterComponent(Entity* entity, Component* component)
{
    /*
    if (component->GetType() == Type::Instance<PrefabComponent>())
    {
        // I am not sure that this is valid behavior. We can potentially delete more than added.
        PrefabComponent* prefabComponent = static_cast<PrefabComponent*>(component);
        entity->RemoveAllChildren();
    }
 */
}

void StreamingSystem::PrepareForRemove()
{
}

void StreamingSystem::LoadLevel(const FilePath& filepath)
{
    level = GetEngineContext()->assetManager->GetAsset<Level>(LevelAssetLoader::StreamLevelKey(filepath), AssetManager::SYNC);
}

void StreamingSystem::StreamEntityCallback(const Asset<AssetBase>& asset, bool reload)
{
    DVASSERT(reload == false); // ??? hmm ???

    /*const Vector<Level::StreamingEvent>& streamingEvents = level->GetActiveStreamingEvents();

    size_t eventCount = streamingEvents.size();
    for (size_t eventIndex = 0; eventIndex < eventCount; ++eventIndex)
    {
        const Level::StreamingEvent& event = streamingEvents[eventIndex];
        if (event.type == Level::StreamingEvent::ENTITY_ADDED)
        {
            GetScene()->AddEntity(event.entity);
        }else if (event.type == Level::StreamingEvent::ENTITY_REMOVED)
        {
            GetScene()->RemoveEntity(event.entity);
        }
    }
    level->ClearActiveStreamingEvents();*/
}

void StreamingSystem::UnloadLevel()
{
    level = nullptr;
}

/*
 Function<void(const Asset<AssetBase>&, bool)> onLoaded;
 Function<void(const Asset<AssetBase>&, bool, const String&)> onError;
 Function<void(const Asset<AssetBase>&)> onUnloaded;
 */

void StreamingSystem::EntityOnLoaded(const Asset<AssetBase>& asset)
{
    Asset<LevelEntity> levelEntity = std::dynamic_pointer_cast<LevelEntity>(asset);
    DVASSERT(levelEntity != nullptr);

    LevelAssetLoader::StreamEntityKey streamEntityKey = levelEntity->GetAssetKey<LevelAssetLoader::StreamEntityKey>();

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

    LevelAssetLoader::StreamEntityKey streamEntityKey = levelEntity->GetAssetKey<LevelAssetLoader::StreamEntityKey>();

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
        //RequestLoadEntity(entityIndex, (uint32)(0xFFFFFFFF));
        Asset<LevelEntity> levelEntity = GetEngineContext()->assetManager->GetAsset<LevelEntity>(LevelAssetLoader::StreamEntityKey(level, entityIndex), AssetManager::ASYNC, &entityAssetListener);

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
            Asset<LevelEntity> levelEntity = GetEngineContext()->assetManager->GetAsset<LevelEntity>(LevelAssetLoader::StreamEntityKey(level, entityIndex), AssetManager::ASYNC, &entityAssetListener);

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

void StreamingSystem::Process(float32 timeElapsed)
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
                        RequestLoadChunk(currentPos);
                    }
                    else
                    {
                        RequestUnloadChunk(currentPos);
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
    }
}
};
