#include "Scene3D/Level.h"
#include "Scene3D/AnimationData.h"
#include "Scene3D/Components/StreamingSettingsComponent.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Scene.h"
#include "Scene3D/SceneFile/SerializationContext.h"
#include "Scene3D/SceneSerialization.h"

#include "Base/BaseMath.h"
#include "Debug/ProfilerCPU.h"
#include "Debug/ProfilerGPU.h"
#include "Debug/ProfilerMarkerNames.h"
#include "Engine/Engine.h"
#include "FileSystem/File.h"
#include "FileSystem/FileSystem.h"
#include "Job/JobManager.h"
#include "Render/3D/PolygonGroup.h"
#include "Render/Material/NMaterial.h"
#include "Time/SystemTimer.h"
#include "Reflection/ReflectionRegistrator.h"

#define DISABLE_LEVEL_STREAMING 1

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(LevelEntity)
{
    ReflectionRegistrator<LevelEntity>::Begin()
    .End();
}

DAVA_VIRTUAL_REFLECTION_IMPL(Level)
{
    ReflectionRegistrator<Level>::Begin()
    .End();
}

LevelEntity::LevelEntity(const Any& assetKey)
    : AssetBase(assetKey)
{
}

LevelEntity::~LevelEntity()
{
    SafeRelease(rootEntity);
}

Level::Level(const Any& assetKey)
    : AssetBase(assetKey)
{
}

Level::Chunk::Chunk(const Level::Chunk& other)
    : entitiesIndices(other.entitiesIndices)
    , entitiesLoaded(other.entitiesLoaded)
    , state(other.state)
    , visitedLastFrameIndex(other.visitedLastFrameIndex)
{
}

Level::Chunk::Chunk(Level::Chunk&& other)
{
    std::swap(entitiesIndices, other.entitiesIndices);
    std::swap(entitiesLoaded, other.entitiesLoaded);
    state = other.state;
    visitedLastFrameIndex = other.visitedLastFrameIndex;
}

Level::Chunk& Level::Chunk::operator=(const Level::Chunk& other)
{
    entitiesIndices = other.entitiesIndices;
    entitiesLoaded = other.entitiesLoaded;
    state = other.state;
    visitedLastFrameIndex = other.visitedLastFrameIndex;
    return *this;
}

Level::Chunk& Level::Chunk::operator=(Level::Chunk&& other)
{
    entitiesIndices = std::move(other.entitiesIndices);
    entitiesLoaded = std::move(other.entitiesLoaded);
    state = other.state;
    visitedLastFrameIndex = other.visitedLastFrameIndex;
    return *this;
}

void Level::ChunkGrid::SetWorldBounds(const AABBox3& newWorldBounds)
{
    if (worldBounds.IsInside(newWorldBounds) == true)
    {
        return;
    }

    Map<Level::ChunkCoord, Chunk> chunks;
    for (int32 y = worldChunkBounds.min.y; y <= worldChunkBounds.max.y; ++y)
    {
        for (int32 x = worldChunkBounds.min.x; x <= worldChunkBounds.max.x; ++x)
        {
            Level::ChunkCoord coord(x, y);
            Level::Chunk* chunk = GetChunk(coord);
            DVASSERT(chunk != nullptr);

            chunks[coord] = std::move(*chunk);
        }
    }

    worldBounds = newWorldBounds;
    worldChunkBounds = ProjectBoxOnGrid(worldBounds);

    chunkXCount = worldChunkBounds.max.x - worldChunkBounds.min.x + 1;
    chunkYCount = worldChunkBounds.max.y - worldChunkBounds.min.y + 1;

    chunkData.resize(chunkXCount * chunkYCount);

    for (auto& node : chunks)
    {
        Level::Chunk* chunk = GetChunk(node.first);
        if (chunk != nullptr)
        {
            *chunk = std::move(node.second);
        }
    }
}

Level::ChunkBounds Level::ChunkGrid::ProjectBoxOnGrid(const AABBox3& entityBox) const
{
    ChunkBounds bounds;

    bounds.min.x = (int32)(std::floor(entityBox.min.x / chunkSize));
    bounds.min.y = (int32)(std::floor(entityBox.min.y / chunkSize));

    bounds.max.x = (int32)(std::floor(entityBox.max.x / chunkSize));
    bounds.max.y = (int32)(std::floor(entityBox.max.y / chunkSize));

    return bounds;
}

Level::ChunkCoord Level::ChunkGrid::GetChunkCoord(const Vector3& position) const
{
    ChunkCoord coord;
    coord.x = (int32)(std::floor(position.x / chunkSize));
    coord.y = (int32)(std::floor(position.y / chunkSize));
    return coord;
}

uint32 Level::ChunkGrid::GetChunkAddress(const ChunkCoord& coord) const
{
    uint32 address = (coord.y - worldChunkBounds.min.y) * chunkXCount + (coord.x - worldChunkBounds.min.x);
    return address;
}

Level::Chunk* Level::ChunkGrid::GetChunk(uint32 address)
{
    if (address < chunkData.size())
        return &chunkData[address];
    return nullptr;
}

Level::Chunk* Level::ChunkGrid::GetChunk(const ChunkCoord& coord)
{
    uint32 address = GetChunkAddress(coord);
    return GetChunk(address);
}

template <>
bool AnyCompare<Level::Key>::IsEqual(const Any& v1, const Any& v2)
{
    return v1.Get<Level::Key>().path == v2.Get<Level::Key>().path;
}

template <>
bool AnyCompare<LevelEntity::Key>::IsEqual(const Any& v1, const Any& v2)
{
    const LevelEntity::Key& left = v1.Get<LevelEntity::Key>();
    const LevelEntity::Key& right = v2.Get<LevelEntity::Key>();

    return left.level == right.level && left.entityIndex == right.entityIndex;
}

} // namespace DAVA
