#pragma once

#include <Base/BaseTypes.h>
#include <Functional/Function.h>
#include <Scene3D/Systems/BaseStreamingSystem.h>
#include <Scene3D/Level.h>

namespace DAVA
{
class Scene;
class FilePath;
} // namespace DAVA

class LevelControllerSystem : public DAVA::BaseStreamingSystem
{
public:
    LevelControllerSystem(DAVA::Scene* scene);

    void RegisterEntity(DAVA::Entity* entity) override;
    void UnregisterEntity(DAVA::Entity* entity) override;
    void RegisterComponent(DAVA::Entity* entity, DAVA::Component* component) override;
    void UnregisterComponent(DAVA::Entity* entity, DAVA::Component* component) override;

    void Process(DAVA::float32 timeElapsed) override;

    void CreateLevel();
    void LoadLevel(const DAVA::FilePath& filepath);
    using TLoadingProgressCallback = DAVA::Function<void(DAVA::uint32 /*loaded*/, DAVA::uint32 /*summary count*/)>;
    void LoadEntities(const TLoadingProgressCallback& callback);

    void SetDrawChunksGrid(bool drawChunkGrid);

protected:
    void ChunkBecomeVisible(const DAVA::Level::ChunkCoord& coord) override;
    void ChunkBecomeInvisible(const DAVA::Level::ChunkCoord& coord) override;

    void RechunkEntities(DAVA::Vector<DAVA::Entity*>& entities);
    DAVA::Entity* GetStreamedEntity(DAVA::Entity* entity) const;
    void UpdateEntityInfoBox(DAVA::Entity* entity, DAVA::Level::EntityInfo& entityInfo) const;
    DAVA::AABBox3 GetEntityBBox(const DAVA::Level::EntityInfo& info) const;
    DAVA::AABBox3 CalcWorldExtents() const;

    void AddToChunk(DAVA::Level::Chunk* chunk, const DAVA::Asset<DAVA::LevelEntity>& levelEntity, DAVA::uint32 entityInfoIndex) const;
    void RemoveFromChunk(DAVA::Level::Chunk* chunk, const DAVA::Asset<DAVA::LevelEntity>& levelEntity, DAVA::uint32 entityInfoIndex) const;
    void ForEachInBounds(const DAVA::Level::ChunkBounds& bounds, const DAVA::Function<void(const DAVA::Level::ChunkCoord& coord)>& callback);

private:
    DAVA::UnorderedMap<DAVA::Asset<DAVA::LevelEntity>, DAVA::int32> visibileInChunkCount;

    struct EntityMappingNode
    {
        DAVA::Asset<DAVA::LevelEntity> asset;
        DAVA::uint32 entityInfoIndex;
    };

    DAVA::UnorderedMap<DAVA::Entity*, EntityMappingNode> entityMapping;
    DAVA::UnorderedSet<DAVA::Entity*> globalChunkEntities;
    DAVA::UnorderedSet<DAVA::uint32> freeEntityInfoIndexes;

    DAVA::UnorderedSet<DAVA::Entity*> pendingRechunk;

    bool inLoadingProcess = false;
};
