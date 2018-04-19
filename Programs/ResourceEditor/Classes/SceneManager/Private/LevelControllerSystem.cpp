#include "Classes/SceneManager/Private/LevelControllerSystem.h"

#include <TArc/Utils/ScopedValueGuard.h>

#include <Engine/Engine.h>
#include <Engine/EngineContext.h>
#include <Math/AABBox3.h>
#include <Math/Vector.h>
#include <Scene3D/Components/SingleComponents/TransformSingleComponent.h>
#include <Scene3D/Components/TransformComponent.h>
#include <Scene3D/Components/StreamingSettingsComponent.h>
#include <Scene3D/Entity.h>
#include <Scene3D/Level.h>
#include <Scene3D/Scene.h>

namespace LevelControllerSystemDetail
{
struct ReChunkEntityInfo
{
    DAVA::Level::ChunkBounds oldBounds;
    DAVA::Level::ChunkBounds newBounds;
    bool addToGlobal = false;
    bool removeFromGlobal = false;
};
} // namespace LevelControllerSystemDetail

LevelControllerSystem::LevelControllerSystem(DAVA::Scene* scene)
    : BaseStreamingSystem(scene)
{
}

void LevelControllerSystem::RegisterEntity(DAVA::Entity* entity)
{
    using namespace DAVA;
    if (entity == GetScene())
    {
        return;
    }

    SCOPED_VALUE_GUARD(bool, inLoadingProcess, true, void());

    Entity* streamedEntity = GetStreamedEntity(entity);
    if (streamedEntity != entity)
    {
        pendingRechunk.insert(streamedEntity);
    }
    else
    {
#if defined(__DAVAENGINE_DEBUG__)
        DVASSERT(entityMapping.find(entity) == entityMapping.end());
#endif

        Asset<Level> level = GetLoadedLevel();
        DVASSERT(level != nullptr);

        level->loadedChunkGrid.SetWorldBounds(CalcWorldExtents());

        EntityMappingNode& mappingNode = entityMapping[entity];

        if (freeEntityInfoIndexes.empty() == false)
        {
            auto iter = freeEntityInfoIndexes.begin();
            mappingNode.entityInfoIndex = *iter;
            freeEntityInfoIndexes.erase(iter);
        }
        else
        {
            mappingNode.entityInfoIndex = static_cast<uint32>(level->loadedInfoArray.size());
            level->loadedInfoArray.push_back(Level::EntityInfo());
        }

        LevelEntity::Key key(level.get(), mappingNode.entityInfoIndex);
        mappingNode.asset = GetEngineContext()->assetManager->CreateAsset<LevelEntity>(key);
        mappingNode.asset->rootEntity = SafeRetain(entity);

        Level::EntityInfo& entityInfo = level->loadedInfoArray[mappingNode.entityInfoIndex];
        UpdateEntityInfoBox(entity, entityInfo);
        if (entity->GetComponent<StreamingSettingsComponent>() != nullptr)
        {
            Level::Chunk* chunk = &level->loadedChunkGrid.specialStreamingSettingsChunk;
            AddToChunk(chunk, mappingNode.asset, mappingNode.entityInfoIndex);
            globalChunkEntities.insert(entity);
        }
        else
        {
            AABBox3 box = GetEntityBBox(entityInfo);

            Level::ChunkBounds bounds = level->loadedChunkGrid.ProjectBoxOnGrid(box);
            ForEachInBounds(bounds, [this, &level, &mappingNode](const Level::ChunkCoord& coord) {
                Level::Chunk* chunk = level->loadedChunkGrid.GetChunk(coord);
                AddToChunk(chunk, mappingNode.asset, mappingNode.entityInfoIndex);
            });
        }
    }
}

void LevelControllerSystem::UnregisterEntity(DAVA::Entity* entity)
{
    using namespace DAVA;
    if (entity == GetScene())
    {
        return;
    }

    Entity* streamedEntity = GetStreamedEntity(entity);
    if (streamedEntity != entity)
    {
        pendingRechunk.insert(streamedEntity);
    }
    else
    {
        Asset<Level> level = GetLoadedLevel();
        auto iter = entityMapping.find(entity);
        DVASSERT(iter != entityMapping.end());
        EntityMappingNode& mappingNode = iter->second;

        if (entity->GetComponent<StreamingSettingsComponent>() != nullptr)
        {
            globalChunkEntities.erase(entity);
            Level::Chunk* chunk = &level->loadedChunkGrid.specialStreamingSettingsChunk;
            RemoveFromChunk(chunk, mappingNode.asset, mappingNode.entityInfoIndex);
        }
        else
        {
            Level::EntityInfo& entityInfo = level->loadedInfoArray[mappingNode.entityInfoIndex];
            AABBox3 entityBox = GetEntityBBox(entityInfo);
            Level::ChunkBounds bounds = level->loadedChunkGrid.ProjectBoxOnGrid(entityBox);

            ForEachInBounds(bounds, [this, &level, &mappingNode](const Level::ChunkCoord& chunkCoord) {
                Level::Chunk* chunk = level->loadedChunkGrid.GetChunk(chunkCoord);
                RemoveFromChunk(chunk, mappingNode.asset, mappingNode.entityInfoIndex);
            });
        }

        freeEntityInfoIndexes.insert(mappingNode.entityInfoIndex);
        visibileInChunkCount.erase(mappingNode.asset);
        entityMapping.erase(iter);
    }
}

void LevelControllerSystem::RegisterComponent(DAVA::Entity* entity, DAVA::Component* component)
{
    pendingRechunk.insert(entity);
}

void LevelControllerSystem::UnregisterComponent(DAVA::Entity* entity, DAVA::Component* component)
{
    pendingRechunk.insert(entity);
}

void LevelControllerSystem::Process(DAVA::float32 timeElapsed)
{
    using namespace DAVA;

    Asset<Level> level = GetLoadedLevel();
    DVASSERT(level != nullptr);

    Scene* scene = GetScene();

    TransformSingleComponent* tsc = scene->GetSingletonComponent<TransformSingleComponent>();
    pendingRechunk.insert(tsc->localTransformChanged.begin(), tsc->localTransformChanged.end());
    if (pendingRechunk.empty() == false)
    {
        Vector<Entity*> transformedEntities(pendingRechunk.begin(), pendingRechunk.end());
        RechunkEntities(transformedEntities);
        pendingRechunk.clear();
    }
    BaseStreamingSystem::Process(timeElapsed);
}

void LevelControllerSystem::CreateLevel()
{
    CreateLevelImpl();
}

void LevelControllerSystem::LoadLevel(const DAVA::FilePath& filepath)
{
    LoadLevelImpl(filepath);
}

void LevelControllerSystem::LoadEntities(const TLoadingProgressCallback& callback)
{
    using namespace DAVA;

    ScopedValueGuard<bool> guard(inLoadingProcess, true);

    Asset<Level> level = GetLoadedLevel();

    auto foreachChunk = [](Level* level, const Function<void(Level::Chunk * chunk)>& fn) {
        fn(&level->loadedChunkGrid.specialStreamingSettingsChunk);

        for (Level::Chunk& chunk : level->loadedChunkGrid.chunkData)
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
    auto loadEntities = [&](Level::Chunk* chunk) {
        uint32 index = 0;
        for (size_t i = 0; i < chunk->entitiesIndices.size(); ++i)
        {
            uint32 entityIndex = chunk->entitiesIndices[i];
            LevelEntity::Key key(level.get(), entityIndex);
            Asset<LevelEntity> levelEntity = assetManager->GetAsset<LevelEntity>(key, AssetManager::SYNC);
            chunk->entitiesLoaded[index++] = levelEntity;
            EntityMappingNode& mappingNode = entityMapping[levelEntity->rootEntity];
            mappingNode.entityInfoIndex = entityIndex;
            mappingNode.asset = levelEntity;
            callback(++loadedCount, entitiesCount);

            levelEntity->rootEntity->SetVisible(false);
        }
    };

    foreachChunk(level.get(), calcEntitiesCount);

    callback(0, entitiesCount);
    foreachChunk(level.get(), loadEntities);

    for (const Asset<LevelEntity>& levelEntity : level->loadedChunkGrid.specialStreamingSettingsChunk.entitiesLoaded)
    {
        globalChunkEntities.emplace(levelEntity->rootEntity);
    }
}

void LevelControllerSystem::SetDrawChunksGrid(bool drawChunkGrid_)
{
    drawChunkGrid = drawChunkGrid_;
}

void LevelControllerSystem::ChunkBecomeVisible(const DAVA::Level::ChunkCoord& coord)
{
    using namespace DAVA;
    Asset<Level> level = GetLoadedLevel();
    uint32 chunkAddress = level->loadedChunkGrid.GetChunkAddress(coord);
    Level::Chunk* chunk = level->loadedChunkGrid.GetChunk(chunkAddress);

    if (chunk->state == Level::Chunk::STATE_NOT_REQUESTED)
    {
        chunk->state = Level::Chunk::STATE_REQUESTED;
        for (const Asset<LevelEntity>& entity : chunk->entitiesLoaded)
        {
            visibileInChunkCount[entity]++;
            entity->rootEntity->SetVisible(true);
        }
    }
}

void LevelControllerSystem::ChunkBecomeInvisible(const DAVA::Level::ChunkCoord& coord)
{
    using namespace DAVA;

    Asset<Level> level = GetLoadedLevel();
    uint32 chunkAddress = level->loadedChunkGrid.GetChunkAddress(coord);
    Level::Chunk* chunk = level->loadedChunkGrid.GetChunk(chunkAddress);

    if (chunk->state == Level::Chunk::STATE_REQUESTED)
    {
        chunk->state = Level::Chunk::STATE_NOT_REQUESTED;
        for (const Asset<LevelEntity>& entity : chunk->entitiesLoaded)
        {
            if (entity->rootEntity->GetVisible() == true)
            {
                int32& refCount = visibileInChunkCount[entity];
                if (refCount > 0)
                {
                    --refCount;
                }

                if (refCount == 0)
                {
                    entity->rootEntity->SetVisible(false);
                }
            }
        }
    }
}

void LevelControllerSystem::RechunkEntities(DAVA::Vector<DAVA::Entity*>& entities)
{
    using namespace DAVA;
    using namespace LevelControllerSystemDetail;

    Scene* scene = GetScene();
    Asset<Level> level = GetLoadedLevel();
    for (size_t i = 0; i < entities.size(); ++i)
    {
        entities[i] = GetStreamedEntity(entities[i]);
    }

    UnorderedMap<Entity*, ReChunkEntityInfo> rechunkEntities;
    for (Entity* entity : entities)
    {
        if (entity != nullptr && rechunkEntities.count(entity) == 0)
        {
            bool addToGlobal = entity->GetComponent<StreamingSettingsComponent>();
            bool removeFromGlobal = globalChunkEntities.count(entity) > 0;

            auto iter = entityMapping.find(entity);
            if (iter == entityMapping.end())
            {
                continue;
            }

            uint32 index = iter->second.entityInfoIndex;
            DVASSERT(index < level->loadedInfoArray.size());
            Level::EntityInfo& entityInfo = level->loadedInfoArray[index];

            AABBox3 oldBox = GetEntityBBox(entityInfo);
            UpdateEntityInfoBox(entity, entityInfo);
            AABBox3 newBox = GetEntityBBox(entityInfo);

            Level::ChunkBounds oldBounds = level->loadedChunkGrid.ProjectBoxOnGrid(oldBox);
            Level::ChunkBounds newBounds = level->loadedChunkGrid.ProjectBoxOnGrid(newBox);

            if (oldBounds != newBounds || addToGlobal != removeFromGlobal)
            {
                rechunkEntities.emplace(entity, ReChunkEntityInfo{ oldBounds, newBounds, addToGlobal, removeFromGlobal });
            }
        }
    }

    if (rechunkEntities.empty())
    {
        return;
    }

    level->loadedChunkGrid.SetWorldBounds(CalcWorldExtents());
    for (const auto& node : rechunkEntities)
    {
        Entity* entity = node.first;
        Level::ChunkBounds oldBounds = node.second.oldBounds;
        Level::ChunkBounds newBounds = node.second.newBounds;

        if (node.second.addToGlobal == true && node.second.removeFromGlobal == true)
        {
            continue;
        }

        Set<Level::ChunkCoord> chunksToRemove;
        Set<Level::ChunkCoord> chunksToAdd;

        if (node.second.addToGlobal == false && node.second.removeFromGlobal == false)
        {
            Set<Level::ChunkCoord> oldChunks;
            Set<Level::ChunkCoord> newChunks;
            ForEachInBounds(oldBounds, [&oldChunks](const Level::ChunkCoord& coord) { oldChunks.insert(coord); });
            ForEachInBounds(newBounds, [&newChunks](const Level::ChunkCoord& coord) { newChunks.insert(coord); });

            std::set_difference(oldChunks.begin(), oldChunks.end(), newChunks.begin(), newChunks.end(), std::inserter(chunksToRemove, chunksToRemove.begin()));
            std::set_difference(newChunks.begin(), newChunks.end(), oldChunks.begin(), oldChunks.end(), std::inserter(chunksToAdd, chunksToAdd.begin()));
        }
        else if (node.second.addToGlobal == true)
        {
            ForEachInBounds(oldBounds, [&chunksToRemove](const Level::ChunkCoord& coord) { chunksToRemove.insert(coord); });
        }
        else if (node.second.removeFromGlobal == true)
        {
            ForEachInBounds(oldBounds, [&chunksToAdd](const Level::ChunkCoord& coord) { chunksToAdd.insert(coord); });
        }

        auto assetIter = entityMapping.find(entity);
        DVASSERT(assetIter != entityMapping.end());
        EntityMappingNode& mappingNode = assetIter->second;

        {
            if (node.second.addToGlobal == false)
            {
                for (const Level::ChunkCoord& coord : chunksToAdd)
                {
                    Level::Chunk* chunk = level->loadedChunkGrid.GetChunk(coord);
                    AddToChunk(chunk, mappingNode.asset, mappingNode.entityInfoIndex);
                }
            }
            else
            {
                Level::Chunk* globalChunk = &level->loadedChunkGrid.specialStreamingSettingsChunk;
                AddToChunk(globalChunk, mappingNode.asset, mappingNode.entityInfoIndex);
                globalChunkEntities.insert(mappingNode.asset->rootEntity);
            }
        }

        {
            if (node.second.removeFromGlobal == false)
            {
                for (const Level::ChunkCoord& coord : chunksToRemove)
                {
                    if (level->loadedChunkGrid.IsInBounds(coord) == false)
                    {
                        continue;
                    }

                    Level::Chunk* chunk = level->loadedChunkGrid.GetChunk(coord);
                    RemoveFromChunk(chunk, mappingNode.asset, mappingNode.entityInfoIndex);
                }
            }
            else
            {
                Level::Chunk* globalChunk = &level->loadedChunkGrid.specialStreamingSettingsChunk;
                RemoveFromChunk(globalChunk, mappingNode.asset, mappingNode.entityInfoIndex);
                globalChunkEntities.erase(mappingNode.asset->rootEntity);
            }
        }
    }
}

DAVA::Entity* LevelControllerSystem::GetStreamedEntity(DAVA::Entity* entity) const
{
    using namespace DAVA;
    Scene* scene = GetScene();
    while (entity != nullptr && entity->GetParent() != scene)
    {
        entity = entity->GetParent();
    }

    return entity;
}

void LevelControllerSystem::UpdateEntityInfoBox(DAVA::Entity* entity, DAVA::Level::EntityInfo& entityInfo) const
{
    using namespace DAVA;
    AABBox3 newBox = entity->GetWTMaximumBoundingBoxSlow();
    if (newBox.IsEmpty())
    {
        TransformComponent* component = entity->GetComponent<TransformComponent>();
        if (component != nullptr)
        {
            AABBox3 defaultBox(Vector3(0.0f, 0.0f, 0.0f), 1.0f);
            defaultBox.GetTransformedBox(component->GetWorldTransform(), newBox);
        }
    }
    entityInfo.boundMinX = static_cast<int16>(std::floor(newBox.min.x));
    entityInfo.boundMinY = static_cast<int16>(std::floor(newBox.min.y));
    entityInfo.boundMinZ = static_cast<int16>(std::floor(newBox.min.z));

    entityInfo.boundMaxX = static_cast<int16>(std::ceil(newBox.max.x));
    entityInfo.boundMaxY = static_cast<int16>(std::ceil(newBox.max.y));
    entityInfo.boundMaxZ = static_cast<int16>(std::ceil(newBox.max.z));
}

DAVA::AABBox3 LevelControllerSystem::GetEntityBBox(const DAVA::Level::EntityInfo& info) const
{
    using namespace DAVA;
    AABBox3 box(Vector3(info.boundMinX, info.boundMinY, info.boundMinZ),
                Vector3(info.boundMaxX, info.boundMaxY, info.boundMaxZ));

    return box;
}

DAVA::AABBox3 LevelControllerSystem::CalcWorldExtents() const
{
    using namespace DAVA;
    Scene* scene = GetScene();
    uint32 size = scene->GetChildrenCount();

    AABBox3 worldMaxExtents;
    for (uint32 index = 0; index < size; ++index)
    {
        AABBox3 box = scene->GetChild(index)->GetWTMaximumBoundingBoxSlow();

        if (box.IsEmpty())
        {
            TransformComponent* transform = scene->GetChild(index)->GetComponent<TransformComponent>();
            if (transform)
            {
                AABBox3 defaultBox(Vector3(0.0f, 0.0f, 0.0f), 1.0f);
                defaultBox.GetTransformedBox(transform->GetWorldTransform(), box);
            }
        }

        worldMaxExtents.AddAABBox(box);
    }

    return worldMaxExtents;
}

void LevelControllerSystem::AddToChunk(DAVA::Level::Chunk* chunk, const DAVA::Asset<DAVA::LevelEntity>& levelEntity, DAVA::uint32 entityInfoIndex) const
{
    DVASSERT(std::find(chunk->entitiesIndices.begin(), chunk->entitiesIndices.end(), entityInfoIndex) == chunk->entitiesIndices.end());
    DVASSERT(std::find(chunk->entitiesLoaded.begin(), chunk->entitiesLoaded.end(), levelEntity) == chunk->entitiesLoaded.end());
    chunk->entitiesIndices.push_back(entityInfoIndex);
    chunk->entitiesLoaded.push_back(levelEntity);
}

void LevelControllerSystem::RemoveFromChunk(DAVA::Level::Chunk* chunk, const DAVA::Asset<DAVA::LevelEntity>& levelEntity, DAVA::uint32 entityInfoIndex) const
{
    auto removeAssetIter = std::find(chunk->entitiesLoaded.begin(), chunk->entitiesLoaded.end(), levelEntity);
    DVASSERT(removeAssetIter != chunk->entitiesLoaded.end());
    size_t offset = std::distance(chunk->entitiesLoaded.begin(), removeAssetIter);
    auto removeInfoIndexIter = std::next(chunk->entitiesIndices.begin(), offset);

    DVASSERT(entityInfoIndex == *removeInfoIndexIter);
    chunk->entitiesLoaded.erase(removeAssetIter);
    chunk->entitiesIndices.erase(removeInfoIndexIter);
}

void LevelControllerSystem::ForEachInBounds(const DAVA::Level::ChunkBounds& bounds, const DAVA::Function<void(const DAVA::Level::ChunkCoord& coord)>& callback)
{
    using namespace DAVA;
    for (int32 y = bounds.min.y; y <= bounds.max.y; ++y)
    {
        for (int32 x = bounds.min.x; x <= bounds.max.x; ++x)
        {
            callback(Level::ChunkCoord(x, y));
        }
    }
}
