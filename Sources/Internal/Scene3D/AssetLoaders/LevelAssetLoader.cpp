#include "Scene3D/AssetLoaders/LevelAssetLoader.h"

#include "Base/BaseTypes.h"
#include "Base/Type.h"
#include "Base/Any.h"
#include "Debug/DVAssert.h"
#include "Scene3D/Level.h"
#include "FileSystem/FilePath.h"
#include "FileSystem/KeyedArchive.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Components/StreamingSettingsComponent.h"
#include "Scene3D/SceneSerialization.h"
#include "Scene3D/SceneFile/VersionInfo.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"
#include "Render/Material/Material.h"
#include "Render/3D/Geometry.h"
#include "Logger/Logger.h"

namespace DAVA
{
namespace LevelAssetLoaderDetail
{
struct Header
{
    enum
    {
        CURRENT_VERSION = 2,
        SUPPORTED_VERSION = 2,
    };

    char8 signature[4] = { 'L', 'V', 'L', 'B' };
    uint32 version = CURRENT_VERSION;
    uint32 allEntitiesCount = 0; // number of entities in whole map
    AABBox3 worldBounds;
};

size_t LevelKeyHash(const Any& v)
{
    const Level::Key& key = v.Get<Level::Key>();
    std::hash<String> hashFn;
    return hashFn(key.path.GetAbsolutePathname());
}

size_t EntityKeyHash(const Any& v)
{
    const LevelEntity::Key& key = v.Get<LevelEntity::Key>();
    std::hash<Level*> levelHash;
    std::hash<uint32> indexHash;

    size_t seed;
    HashCombine(seed, key.level);
    HashCombine(seed, key.entityIndex);

    return seed;
}

AABBox3 ComputeEntitiesBoxesAndWorldExtents(Scene* scene, Vector<AABBox3>& boxes)
{
    AABBox3 worldMaxExtents;
    uint32 size = scene->GetChildrenCount();
    boxes.resize(size);
    Logger::Debug<Level>("Compute bboxes: %d", boxes.size());
    for (uint32 index = 0; index < size; ++index)
    {
        boxes[index] = scene->GetChild(index)->GetWTMaximumBoundingBoxSlow();

        AABBox3& box = boxes[index];
        if ((AABBOX_INFINITY == box.min.x && AABBOX_INFINITY == box.min.y && AABBOX_INFINITY == box.min.z)
            && (-AABBOX_INFINITY == box.max.x && -AABBOX_INFINITY == box.max.y && -AABBOX_INFINITY == box.max.z))
        {
            TransformComponent* transform = scene->GetChild(index)->GetComponent<TransformComponent>();
            if (transform)
            {
                Vector3 translationVector = transform->GetWorldTransform().GetTranslationVector();
                box.AddPoint(translationVector);
            }
        }

        worldMaxExtents.AddAABBox(boxes[index]);
        Logger::Debug<Level>("%0.1f, %0.1f, %0.1f, %0.1f, %0.1f, %0.1f",
                             boxes[index].min.x, boxes[index].min.y, boxes[index].min.z,
                             boxes[index].max.x, boxes[index].max.y, boxes[index].max.z);
    }
    return worldMaxExtents;
}

void WriteChunkTable(File* file,
                     Scene* scene,
                     Level::ChunkGrid& chunkGrid,
                     const Vector<AABBox3>& entitiesBoxes)
{
    file->Write(&chunkGrid.worldChunkBounds, sizeof(chunkGrid.worldChunkBounds));
    file->Write(&chunkGrid.chunkXCount, sizeof(chunkGrid.chunkXCount));
    file->Write(&chunkGrid.chunkYCount, sizeof(chunkGrid.chunkYCount));
    file->Write(&chunkGrid.worldBounds, sizeof(chunkGrid.worldBounds));

    // Populate chunks with data
    uint32 maxEntitiesInChunk = 0;

    size_t entityCount = entitiesBoxes.size();
    for (size_t entityIndex = 0; entityIndex < entityCount; ++entityIndex)
    {
        Entity* entity = scene->GetChild(int32(entityIndex));
        StreamingSettingsComponent* streamingSettingsComponent = entity->GetComponent<StreamingSettingsComponent>();
        if (streamingSettingsComponent)
        {
            chunkGrid.specialStreamingSettingsChunk.entitiesIndices.emplace_back(uint32(entityIndex));
        }
        else
        {
            Level::ChunkBounds bounds = chunkGrid.ProjectBoxOnGrid(entitiesBoxes[entityIndex]);

            for (int32 y = bounds.min.y; y <= bounds.max.y; ++y)
                for (int32 x = bounds.min.x; x <= bounds.max.x; ++x)
                {
                    Level::Chunk* chunk = chunkGrid.GetChunk(Level::ChunkCoord(x, y));
                    chunk->entitiesIndices.emplace_back(uint32(entityIndex));

                    maxEntitiesInChunk = Max(maxEntitiesInChunk, uint32(chunk->entitiesIndices.size()));
                }
            Logger::Debug<Level>("bounds: %d, %d, %d, %d",
                                 bounds.min.x, bounds.min.y,
                                 bounds.max.x, bounds.max.y);
        }
    }
    Logger::Debug<Level>("max: %d", maxEntitiesInChunk);

    file->Write(&maxEntitiesInChunk, sizeof(maxEntitiesInChunk));

    {
        Level::Chunk* chunk = &chunkGrid.specialStreamingSettingsChunk;
        uint32* data = chunk->entitiesIndices.data();
        uint16 cnt = uint16(chunk->entitiesIndices.size());
        file->Write(&cnt, sizeof(uint16));
        if (cnt > 0)
            file->Write(data, sizeof(uint32) * cnt);
    }

    // Write chunks to disk
    for (int32 y = chunkGrid.worldChunkBounds.min.y; y <= chunkGrid.worldChunkBounds.max.y; ++y)
        for (int32 x = chunkGrid.worldChunkBounds.min.x; x <= chunkGrid.worldChunkBounds.max.x; ++x)
        {
            Level::Chunk* chunk = chunkGrid.GetChunk(Level::ChunkCoord(x, y));
            Logger::Debug<Level>("objs: %d, %d, %d", x, y, chunk->entitiesIndices.size());
            DVASSERT(chunk != nullptr);
            DVASSERT(chunk->entitiesIndices.size() <= maxEntitiesInChunk);
            uint32* data = chunk->entitiesIndices.data();
            uint16 cnt = uint16(chunk->entitiesIndices.size());
            file->Write(&cnt, sizeof(uint16));
            if (cnt > 0)
                file->Write(data, sizeof(uint32) * cnt);
        }
}

void WriteEntities(File* file, Scene* scene, SerializationContext* serializationContext, const Vector<AABBox3>& entitiesBoxes)
{
    DVASSERT(sizeof(Level::EntityInfo) == 20);

    size_t entityCount = entitiesBoxes.size();
    uint64 entityInfoTableStartPos = file->GetPos();
    uint32 entityInfoTableSize = uint32(sizeof(Level::EntityInfo) * entityCount);

    // Pass the table
    file->Seek(entityInfoTableSize, File::SEEK_FROM_CURRENT);

    Vector<Level::EntityInfo> infoArray(entityCount);
    // Save entities and record entity info offsets

    for (size_t entityIndex = 0; entityIndex < entityCount; ++entityIndex)
    {
        Entity* entity = scene->GetChild(int32(entityIndex));
        KeyedArchive* archive = new KeyedArchive();
        SceneSerialization::SaveHierarchy(entity, archive, serializationContext, SceneSerialization::LEVEL);

        Level::EntityInfo& entityInfo = infoArray[entityIndex];
        entityInfo.fileOffset = uint32(file->GetPos());
        archive->Save(file);
        entityInfo.fileSize = uint32(file->GetPos()) - entityInfo.fileOffset;

        SafeRelease(archive);
    }

    // Build entity info table
    for (size_t entityIndex = 0; entityIndex < entityCount; ++entityIndex)
    {
        Level::EntityInfo& entityInfo = infoArray[entityIndex];

        const AABBox3& box = entitiesBoxes[entityIndex];
        entityInfo.boundMinX = (int16)box.min.x;
        entityInfo.boundMinY = (int16)box.min.y;
        entityInfo.boundMinZ = (int16)box.min.z;

        entityInfo.boundMaxX = (int16)box.max.x;
        entityInfo.boundMaxX = (int16)box.max.y;
        entityInfo.boundMaxZ = (int16)box.max.z;
    }

    // Write entityInfo table in right position.
    file->Seek(entityInfoTableStartPos, File::SEEK_FROM_START);
    file->Write(infoArray.data(), entityInfoTableSize);
}

void ReadChunkTable(File* file,
                    Asset<Level> level,
                    const AABBox3& worldBounds_)
{
    level->loadedChunkGrid = new Level::ChunkGrid(worldBounds_);
    Level::ChunkGrid& chunkGrid = *level->loadedChunkGrid;

    Level::ChunkBounds worldChunkBounds;
    uint32 chunkXCount;
    uint32 chunkYCount;
    AABBox3 worldBounds;

    file->Read(&worldChunkBounds, sizeof(chunkGrid.worldChunkBounds));
    file->Read(&chunkXCount, sizeof(chunkGrid.chunkXCount));
    file->Read(&chunkYCount, sizeof(chunkGrid.chunkYCount));
    file->Read(&worldBounds, sizeof(chunkGrid.worldBounds));

    DVASSERT(chunkGrid.chunkXCount == chunkXCount);
    DVASSERT(chunkGrid.chunkYCount == chunkYCount);

    // Populate chunks with data
    uint32 maxEntitiesInChunk = 0;
    file->Read(&maxEntitiesInChunk, sizeof(maxEntitiesInChunk));

    {
        Level::Chunk* chunk = &chunkGrid.specialStreamingSettingsChunk;
        uint16 cnt = 0;
        file->Read(&cnt, sizeof(uint16));
        chunk->entitiesIndices.resize(cnt);
        chunk->entitiesLoaded.resize(cnt);
        if (cnt > 0)
        {
            file->Read(chunk->entitiesIndices.data(), sizeof(uint32) * cnt);
        }
    }

    // Write chunks to disk
    for (int32 y = chunkGrid.worldChunkBounds.min.y; y <= chunkGrid.worldChunkBounds.max.y; ++y)
        for (int32 x = chunkGrid.worldChunkBounds.min.x; x <= chunkGrid.worldChunkBounds.max.x; ++x)
        {
            Level::Chunk* chunk = chunkGrid.GetChunk(Level::ChunkCoord(x, y));
            uint16 cnt = 0;
            file->Read(&cnt, sizeof(uint16));
            chunk->entitiesIndices.resize(cnt);
            chunk->entitiesLoaded.resize(cnt);
            if (cnt > 0)
            {
                file->Read(chunk->entitiesIndices.data(), sizeof(uint32) * cnt);
            }
        }
}

void ReadEntitiesTable(File* file, Asset<Level> level, uint32 entityCount)
{
    DVASSERT(sizeof(Level::EntityInfo) == 20);

    uint32 entityInfoTableSize = sizeof(Level::EntityInfo) * entityCount;
    level->loadedInfoArray.resize(entityCount);
    file->Read(level->loadedInfoArray.data(), entityInfoTableSize);
}

void ReadAllEntities(File* file, Asset<Level> level, SerializationContext* serializationContext, uint32 entityCount)
{
    DVASSERT(0 && "not finished yet");

    for (size_t entityIndex = 0; entityIndex < entityCount; ++entityIndex)
    {
        Level::EntityInfo& entityInfo = level->loadedInfoArray[entityIndex];

        file->Seek(entityInfo.fileOffset, File::SEEK_FROM_START);
        KeyedArchive* archive = new KeyedArchive();
        archive->Load(file);

        Entity* entity = SceneSerialization::LoadHierarchy(nullptr, archive, serializationContext, SceneSerialization::LEVEL);

        SafeRelease(archive);
    }
}

} // namespace LevelAssetLoaderDetail

LevelAssetLoader::LevelAssetLoader()
{
    AnyHash<Level::Key>::Register(&LevelAssetLoaderDetail::LevelKeyHash);
    AnyHash<LevelEntity::Key>::Register(&LevelAssetLoaderDetail::EntityKeyHash);
}

AssetFileInfo LevelAssetLoader::GetAssetFileInfo(const Any& assetKey) const
{
    AssetFileInfo info;
    if (assetKey.CanGet<Level::Key>())
    {
        const Level::Key& key = assetKey.Get<Level::Key>();
        info.fileName = key.path.GetAbsolutePathname();
    }
    else if (assetKey.CanGet<LevelEntity::Key>())
    {
        const LevelEntity::Key& key = assetKey.Get<LevelEntity::Key>();
        info.fileName = Format("ID %u", key.entityIndex);
    }
    info.inMemoryAsset = true;
    return info;
}

AssetBase* LevelAssetLoader::CreateAsset(const Any& assetKey) const
{
    AssetBase* asset = nullptr;
    if (assetKey.CanGet<Level::Key>())
    {
        asset = new Level(assetKey);
    }
    else if (assetKey.CanGet<LevelEntity::Key>())
    {
        asset = new LevelEntity(assetKey);
    }
    DVASSERT(asset != nullptr, "Invalid Asset Key");
    return asset;
}

void LevelAssetLoader::DeleteAsset(AssetBase* asset) const
{
    delete asset;
}

void LevelAssetLoader::LoadAsset(Asset<AssetBase> asset, File* /*file*/, bool reloading, String& errorMessage) const
{
    using namespace LevelAssetLoaderDetail;
    DVASSERT(reloading == false);

    const Any& assetKey = asset->GetAssetKey();
    if (assetKey.CanGet<Level::Key>())
    {
        const Level::Key& levelKey = assetKey.Get<Level::Key>();
        Asset<Level> level = std::dynamic_pointer_cast<Level>(asset);
        level->levelFile = RefPtr<File>(File::Create(levelKey.path.GetAbsolutePathname(), File::OPEN | File::READ));

        Header header;
        level->levelFile->Read(&header, sizeof(Header));

        if (header.version < Header::SUPPORTED_VERSION)
        {
            errorMessage = "Unsupported version of .level file.";
            return;
        }
        SerializationContext serializationContext;
        serializationContext.SetVersion(STREAMING_SCENE_VERSION);
        serializationContext.SetScenePath(level->levelFile->GetFilename().GetDirectory());

        LevelAssetLoaderDetail::ReadChunkTable(level->levelFile.Get(), level, header.worldBounds);
        LevelAssetLoaderDetail::ReadEntitiesTable(level->levelFile.Get(), level, header.allEntitiesCount);
    }
    else if (assetKey.CanGet<LevelEntity::Key>())
    {
        Asset<LevelEntity> levelEntity = std::dynamic_pointer_cast<LevelEntity>(asset);
        LevelEntity::Key entityKey = levelEntity->GetAssetKey<LevelEntity::Key>();

        Level::EntityInfo& entityInfo = entityKey.level->loadedInfoArray[entityKey.entityIndex];
        entityKey.level->levelFile->Seek(entityInfo.fileOffset, File::SEEK_FROM_START);

        KeyedArchive* archive = new KeyedArchive();
        archive->Load(entityKey.level->levelFile.Get());

        SerializationContext serializationContext;
        serializationContext.SetVersion(STREAMING_SCENE_VERSION);
        serializationContext.SetScenePath(entityKey.level->levelFile->GetFilename().GetDirectory());
        Entity* entity = SceneSerialization::LoadHierarchy(nullptr, archive, &serializationContext, SceneSerialization::LEVEL);
        DVASSERT(levelEntity->rootEntity == nullptr);
        levelEntity->rootEntity = entity;
        SafeRelease(archive);
    }
}

bool LevelAssetLoader::SaveAsset(Asset<AssetBase> asset, File* file, eSaveMode /*requestedMode*/) const
{
    return false;
}

bool LevelAssetLoader::SaveAssetFromData(const Any& data, File* file, eSaveMode /*requestedMode*/) const
{
    SerializationContext serializationContext;
    serializationContext.SetScenePath(file->GetFilename().GetDirectory());
    serializationContext.SetVersion(STREAMING_SCENE_VERSION);

    if (!data.CanGet<Scene*>())
        return false;

    Scene* scene = data.Get<Scene*>();
    uint32 childrenCount = scene->GetChildrenCount();

    Level::Header currentHeader;
    currentHeader.allEntitiesCount = scene->GetChildrenCount();

    Vector<AABBox3> entitiesBoxes;
    AABBox3 worldBounds = LevelAssetLoaderDetail::ComputeEntitiesBoxesAndWorldExtents(scene, entitiesBoxes);

    Level::ChunkGrid chunkGrid(worldBounds);
    currentHeader.worldBounds = worldBounds;

    file->Write(&currentHeader, sizeof(Level::Header));

    LevelAssetLoaderDetail::WriteChunkTable(file, scene, chunkGrid, entitiesBoxes);
    LevelAssetLoaderDetail::WriteEntities(file, scene, &serializationContext, entitiesBoxes);

    return true;
}

Vector<const Type*> LevelAssetLoader::GetAssetKeyTypes() const
{
    return Vector<const Type*>{ Type::Instance<Level::Key>(), Type::Instance<LevelEntity::Key>() };
}

Vector<String> LevelAssetLoader::GetDependsOnFiles(const AssetBase* asset) const
{
    // STREAMING_COMPLETE - we need to discuss this
    /*const Any& assetKey = asset->GetKey();
    if (assetKey.CanGet<LevelAssetLoader::FullLevelKey>())
    {
        const LevelAssetLoader::FullLevelKey& key = assetKey.Get<LevelAssetLoader::FullLevelKey>();
        return Vector<String>{ key.path.GetAbsolutePathname() };
    }
    else if (assetKey.CanGet<LevelAssetLoader::StreamLevelKey>())
    {
        const LevelAssetLoader::StreamLevelKey& key = assetKey.Get<LevelAssetLoader::StreamLevelKey>();
        return Vector<String>{ key.path.GetAbsolutePathname() };
    }
    else if (assetKey.CanGet<LevelAssetLoader::StreamEntityKey>())
    {
        const LevelAssetLoader::StreamEntityKey& key = assetKey.Get<LevelAssetLoader::StreamEntityKey>();
        return Vector<String>{};
    }
    DVASSERT(0 && "Wrong asset");*/
    return Vector<String>{};
}
} // namespace DAVA
