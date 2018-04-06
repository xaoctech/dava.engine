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

    using TLoadingProgressCallback = DAVA::Function<void(DAVA::uint32 /*loaded*/, DAVA::uint32 /*summary count*/)>;
    void LoadLevel(const DAVA::FilePath& filepath, const TLoadingProgressCallback& callback);

protected:
    void ChunkBecomeVisible(const DAVA::Level::ChunkCoord& coord) override;
    void ChunkBecomeInvisible(const DAVA::Level::ChunkCoord& coord) override;

private:
    DAVA::UnorderedMap<DAVA::Asset<DAVA::LevelEntity>, DAVA::int32> visibileInChunkCount;
    DAVA::UnorderedMap<DAVA::Entity*, DAVA::uint32> entityToInfoIndex;
    DAVA::UnorderedMap<DAVA::Entity*, DAVA::Asset<DAVA::LevelEntity>> entityToAsset;
    DAVA::UnorderedSet<DAVA::uint32> freeEntityInfoIndexes;
};
