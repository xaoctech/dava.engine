#include "Classes/SceneManager/Private/LevelControllerSystem.h"

#include <Engine/Engine.h>
#include <Engine/EngineContext.h>
#include <Math/AABBox3.h>
#include <Math/Vector.h>
#include <Scene3D/Components/SingleComponents/TransformSingleComponent.h>
#include <Scene3D/Components/TransformComponent.h>
#include <Scene3D/Entity.h>
#include <Scene3D/Level.h>
#include <Scene3D/Scene.h>

namespace LevelControllerSystemDetail
{
struct ReChunkEntityInfo
{
    DAVA::Level::ChunkBounds oldBounds;
    DAVA::Level::ChunkBounds newBounds;
    DAVA::uint32 entityIndex = 0;
};
} // namespace LevelControllerSystemDetail

LevelControllerSystem::LevelControllerSystem(DAVA::Scene* scene)
    : BaseStreamingSystem(scene)
{
}

void LevelControllerSystem::RegisterEntity(DAVA::Entity* entity)
{
}

void LevelControllerSystem::UnregisterEntity(DAVA::Entity* entity)
{
}

void LevelControllerSystem::RegisterComponent(DAVA::Entity* entity, DAVA::Component* component)
{
}

void LevelControllerSystem::UnregisterComponent(DAVA::Entity* entity, DAVA::Component* component)
{
}

void LevelControllerSystem::Process(DAVA::float32 timeElapsed)
{
    using namespace DAVA;
    using namespace LevelControllerSystemDetail;

    SCOPE_EXIT
    {
        BaseStreamingSystem::Process(timeElapsed);
    };

    Asset<Level> level = GetLoadedLevel();
    DVASSERT(level != nullptr);

    Scene* scene = GetScene();
    UnorderedMap<Entity*, ReChunkEntityInfo> rechunkEntities;

    TransformSingleComponent* tsc = scene->GetSingletonComponent<TransformSingleComponent>();
    Vector<Entity*> transformedEntities = tsc->localTransformChanged;
    std::sort(transformedEntities.begin(), transformedEntities.end());
    transformedEntities.erase(std::unique(transformedEntities.begin(), transformedEntities.end()), transformedEntities.end());
    for (Entity* entity : transformedEntities)
    {
        while (entity != nullptr && entity->GetParent() != scene)
        {
            entity = entity->GetParent();
        }

        if (entity != nullptr && rechunkEntities.count(entity) == 0)
        {
            auto iter = entityToInfoIndex.find(entity);
            if (iter == entityToInfoIndex.end())
            {
                continue;
            }

            uint32 index = iter->second;
            DVASSERT(index < level->loadedInfoArray.size());
            Level::EntityInfo& entityInfo = level->loadedInfoArray[index];
            AABBox3 oldBox(Vector3(entityInfo.boundMinX, entityInfo.boundMinY, entityInfo.boundMinZ),
                           Vector3(entityInfo.boundMaxX, entityInfo.boundMaxY, entityInfo.boundMaxZ));

            AABBox3 newBox = entity->GetWTMaximumBoundingBoxSlow();
            entityInfo.boundMinX = static_cast<int16>(std::floor(newBox.min.x));
            entityInfo.boundMinY = static_cast<int16>(std::floor(newBox.min.y));
            entityInfo.boundMinZ = static_cast<int16>(std::floor(newBox.min.z));

            entityInfo.boundMaxX = static_cast<int16>(std::ceil(newBox.max.x));
            entityInfo.boundMaxY = static_cast<int16>(std::ceil(newBox.max.y));
            entityInfo.boundMaxZ = static_cast<int16>(std::ceil(newBox.max.z));

            Level::ChunkBounds oldBounds = level->loadedChunkGrid->ProjectBoxOnGrid(oldBox);
            Level::ChunkBounds newBounds = level->loadedChunkGrid->ProjectBoxOnGrid(newBox);
            if (oldBounds != newBounds)
            {
                rechunkEntities.emplace(entity, ReChunkEntityInfo{ oldBounds, newBounds, index });
            }
        }
    }

    if (rechunkEntities.empty())
    {
        return;
    }

    AABBox3 worldMaxExtents;
    uint32 size = scene->GetChildrenCount();
    for (uint32 index = 0; index < size; ++index)
    {
        AABBox3 box = scene->GetChild(index)->GetWTMaximumBoundingBoxSlow();

        if (box.IsEmpty())
        {
            TransformComponent* transform = scene->GetChild(index)->GetComponent<TransformComponent>();
            if (transform)
            {
                Vector3 translationVector = transform->GetWorldTransform().GetTranslationVector();
                box.AddPoint(translationVector);
            }
        }

        worldMaxExtents.AddAABBox(box);
    }

    auto foreachChunk = [](const Level::ChunkBounds& bounds, const Function<void(const Level::ChunkCoord&)>& fn) {
        for (int32 y = bounds.min.y; y <= bounds.max.y; ++y)
        {
            for (int32 x = bounds.min.x; x <= bounds.max.x; ++x)
            {
                fn(Level::ChunkCoord(x, y));
            }
        }
    };

    level->loadedChunkGrid->SetWorldBounds(worldMaxExtents);
    for (const auto& node : rechunkEntities)
    {
        Entity* entity = node.first;
        Level::ChunkBounds oldBounds = node.second.oldBounds;
        Level::ChunkBounds newBounds = node.second.newBounds;
        uint32 index = node.second.entityIndex;

        Set<Level::ChunkCoord> oldChunks;
        Set<Level::ChunkCoord> newChunks;
        foreachChunk(oldBounds, [&oldChunks](const Level::ChunkCoord& coord) { oldChunks.insert(coord); });
        foreachChunk(newBounds, [&newChunks](const Level::ChunkCoord& coord) { newChunks.insert(coord); });

        Set<Level::ChunkCoord> chunksToRemove;
        Set<Level::ChunkCoord> chunksToAdd;

        std::set_difference(oldChunks.begin(), oldChunks.end(), newChunks.begin(), newChunks.end(), std::inserter(chunksToRemove, chunksToRemove.begin()));
        std::set_difference(newChunks.begin(), newChunks.end(), oldChunks.begin(), oldChunks.end(), std::inserter(chunksToAdd, chunksToAdd.begin()));

        for (const Level::ChunkCoord& coord : chunksToAdd)
        {
            Level::Chunk* chunk = level->loadedChunkGrid->GetChunk(coord);
            DVASSERT(std::find(chunk->entitiesIndices.begin(), chunk->entitiesIndices.end(), index) == chunk->entitiesIndices.end());
            chunk->entitiesIndices.push_back(index);

            auto assetIter = entityToAsset.find(entity);
            DVASSERT(assetIter != entityToAsset.end());

            chunk->entitiesLoaded.push_back(assetIter->second);
        }

        for (const Level::ChunkCoord& coord : chunksToRemove)
        {
            Level::Chunk* chunk = level->loadedChunkGrid->GetChunk(coord);
            for (size_t i = 0; i < chunk->entitiesLoaded.size(); ++i)
            {
                if (chunk->entitiesLoaded[i]->rootEntity == entity)
                {
                    DVASSERT(chunk->entitiesIndices[i] == index);
                    chunk->entitiesLoaded.erase(std::next(chunk->entitiesLoaded.begin(), i));
                    chunk->entitiesIndices.erase(std::next(chunk->entitiesIndices.begin(), i));
                    break;
                }
            }
        }
    }
}

void LevelControllerSystem::LoadLevel(const DAVA::FilePath& filepath, const TLoadingProgressCallback& callback)
{
    using namespace DAVA;

    LoadLevelImpl(filepath);
    Asset<Level> level = GetLoadedLevel();

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
    auto loadEntities = [&](Level::Chunk* chunk) {
        uint32 index = 0;
        for (uint32 entityIndex : chunk->entitiesIndices)
        {
            LevelEntity::Key key(level.get(), entityIndex);
            Asset<LevelEntity> levelEntity = assetManager->GetAsset<LevelEntity>(key, AssetManager::SYNC);
            chunk->entitiesLoaded[index++] = levelEntity;
            entityToInfoIndex[levelEntity->rootEntity] = entityIndex;
            entityToAsset[levelEntity->rootEntity] = levelEntity;
            callback(++loadedCount, entitiesCount);

            levelEntity->rootEntity->SetVisible(false);
        }
    };

    foreachChunk(level.get(), calcEntitiesCount);

    callback(0, entitiesCount);
    foreachChunk(level.get(), loadEntities);
}

void LevelControllerSystem::ChunkBecomeVisible(const DAVA::Level::ChunkCoord& coord)
{
    using namespace DAVA;
    Asset<Level> level = GetLoadedLevel();
    uint32 chunkAddress = level->loadedChunkGrid->GetChunkAddress(coord);
    Level::Chunk* chunk = level->loadedChunkGrid->GetChunk(chunkAddress);

    for (const Asset<LevelEntity>& entity : chunk->entitiesLoaded)
    {
        visibileInChunkCount[entity]++;
        entity->rootEntity->SetVisible(true);
    }
}

void LevelControllerSystem::ChunkBecomeInvisible(const DAVA::Level::ChunkCoord& coord)
{
    using namespace DAVA;

    Asset<Level> level = GetLoadedLevel();
    uint32 chunkAddress = level->loadedChunkGrid->GetChunkAddress(coord);
    Level::Chunk* chunk = level->loadedChunkGrid->GetChunk(chunkAddress);

    for (const Asset<LevelEntity>& entity : chunk->entitiesLoaded)
    {
        if (entity->rootEntity->GetVisible() == true)
        {
            int32& refCount = visibileInChunkCount[entity];
            DVASSERT(refCount > 0);
            --refCount;
            if (refCount == 0)
            {
                entity->rootEntity->SetVisible(false);
            }
        }
    }
}
