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

template <class T>
size_t PathKeyHash(const Any& v)
{
    const T& key = v.Get<T>();
    std::hash<String> hashFn;
    return hashFn(key.path.GetAbsolutePathname());
}

size_t StreamEntityKeyHash(const Any& v)
{
    const LevelAssetLoader::StreamEntityKey& key = v.Get<LevelAssetLoader::StreamEntityKey>();
    std::hash<Asset<Level>> levelHash;
    std::hash<uint32> indexHash;

    return levelHash(key.level) ^ (indexHash(key.entityIndex) << 1);
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

    /*
    auto GetOwnerSet = [&](DataNode* node) -> String
   {
       String res = "";
       for (const FastName& owner : ownerSet[node])
       {
           res += String(owner.c_str());
       }
       return res;
   };
    */

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
        /** Debug info **/
        //       Set<DataNode*> dataNodesInEntity;
        //       entity->GetDataNodes(dataNodesInEntity);
        //       Logger::Debug<Level>("Entity: %s DataNodes Count: %d", entity->GetName().c_str(), dataNodesInEntity.size());
        //       for (DataNode* node : dataNodesInEntity)
        //       {
        //           PolygonGroup* geometry = dynamic_cast<PolygonGroup*>(node);
        //           if (geometry)
        //           {
        //               Logger::Debug<Level>("-- Geometry: %s vertices: %d, primitives: %d owners: %s", dataNodesWithNames[node].GetFilename().c_str(),
        //                                    geometry->GetVertexCount(), geometry->GetPrimitiveCount(), GetOwnerSet(node).c_str());
        //           }
        //           else
        //               Logger::Debug<Level>("-- DataNode: %s owners: %s", dataNodesWithNames[node].GetFilename().c_str(), GetOwnerSet(node).c_str());
        //       }
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
    AnyHash<LevelAssetLoader::FullLevelKey>::Register(&LevelAssetLoaderDetail::PathKeyHash<LevelAssetLoader::FullLevelKey>);
    AnyHash<LevelAssetLoader::StreamLevelKey>::Register(&LevelAssetLoaderDetail::PathKeyHash<LevelAssetLoader::StreamLevelKey>);
    AnyHash<LevelAssetLoader::StreamEntityKey>::Register(&LevelAssetLoaderDetail::StreamEntityKeyHash);
}

AssetFileInfo LevelAssetLoader::GetAssetFileInfo(const Any& assetKey) const
{
    AssetFileInfo info;
    if (assetKey.CanGet<LevelAssetLoader::FullLevelKey>())
    {
        const auto& key = assetKey.Get<LevelAssetLoader::FullLevelKey>();
        info.fileName = key.path.GetAbsolutePathname();
    }
    else if (assetKey.CanGet<LevelAssetLoader::StreamLevelKey>())
    {
        const auto& key = assetKey.Get<LevelAssetLoader::StreamLevelKey>();
        info.fileName = key.path.GetAbsolutePathname();
    }
    else if (assetKey.CanGet<LevelAssetLoader::StreamEntityKey>())
    {
        const auto& key = assetKey.Get<LevelAssetLoader::StreamEntityKey>();
        info.inMemoryAsset = true;
    }
    return info;
}

AssetBase* LevelAssetLoader::CreateAsset(const Any& assetKey) const
{
    if (assetKey.CanGet<LevelAssetLoader::FullLevelKey>())
    {
        return new Level(assetKey);
    }
    else if (assetKey.CanGet<LevelAssetLoader::StreamLevelKey>())
    {
        return new Level(assetKey);
    }
    else if (assetKey.CanGet<LevelAssetLoader::StreamEntityKey>())
    {
        return new LevelEntity(assetKey);
    }
    DVASSERT(0 && "Invalid Asset Key");
    return nullptr;
}

void LevelAssetLoader::DeleteAsset(AssetBase* asset) const
{
    DVASSERT(dynamic_cast<Level*>(asset) != nullptr);
    delete asset;
}

void LevelAssetLoader::LoadAsset(Asset<AssetBase> asset, File* file, bool reloading, String& errorMessage) const
{
    using namespace LevelAssetLoaderDetail;

    const Any& assetKey = asset->GetAssetKey();
    if (!assetKey.CanGet<LevelAssetLoader::StreamEntityKey>())
    {
        Asset<Level> level = std::dynamic_pointer_cast<Level>(asset);

        Header header;
        file->Read(&header, sizeof(Header));

        if (header.version < Header::SUPPORTED_VERSION)
        {
            errorMessage = "Unsupported version of .level file.";
            return;
        }
        SerializationContext serializationContext;
        serializationContext.SetVersion(STREAMING_SCENE_VERSION);
        serializationContext.SetScenePath(file->GetFilename().GetDirectory());

        LevelAssetLoaderDetail::ReadChunkTable(file, level, header.worldBounds);
        LevelAssetLoaderDetail::ReadEntitiesTable(file, level, header.allEntitiesCount);

        if (assetKey.CanGet<LevelAssetLoader::FullLevelKey>())
        {
            LevelAssetLoaderDetail::ReadAllEntities(file, level, &serializationContext, header.allEntitiesCount);
        }
        else if (assetKey.CanGet<LevelAssetLoader::StreamLevelKey>())
        {
            /* save reference to file * to avoid it's closing */
            StreamLevelKey levelKey = assetKey.Get<StreamLevelKey>();

            // retain here
            levelKey.file = file;
        }
    }
    else if (assetKey.CanGet<LevelAssetLoader::StreamEntityKey>())
    {
        Asset<LevelEntity> levelEntity = std::dynamic_pointer_cast<LevelEntity>(asset);
        StreamEntityKey streamEntityKey = levelEntity->GetAssetKey<StreamEntityKey>();
        StreamLevelKey levelKey = streamEntityKey.level->GetAssetKey<StreamLevelKey>();

        Level::EntityInfo& entityInfo = streamEntityKey.level->loadedInfoArray[streamEntityKey.entityIndex];
        levelKey.file->Seek(entityInfo.fileOffset, File::SEEK_FROM_START);

        KeyedArchive* archive = new KeyedArchive();
        archive->Load(levelKey.file.Get());

        Entity* entity = SceneSerialization::LoadHierarchy(nullptr, archive,
                                                           nullptr,
                                                           SceneSerialization::LEVEL);
        DVASSERT(levelEntity->rootEntity == nullptr);
        levelEntity->rootEntity = entity;
        SafeRelease(archive);
    }
}

bool LevelAssetLoader::SaveAsset(Asset<AssetBase> asset, File* file, eSaveMode /*requestedMode*/) const
{
    /*
     using namespace LevelAssetLoaderDetail;
    Asset<Level> level = std::dynamic_pointer_cast<Level>(asset);
    FullLevelKey levelKey = level->GetAssetKey<FullLevelKey>();
    Scene* scene = levelKey.scene;
    
    SerializationContext serializationContext;
    serializationContext.SetScenePath(file->GetFilename().GetDirectory());
    serializationContext.SetVersion(STREAMING_SCENE_VERSION);
    
    Header currentHeader;
    currentHeader.allEntitiesCount = scene->GetChildrenCount();
        
    Vector<AABBox3> entitiesBoxes;
    AABBox3 worldBounds = LevelAssetLoaderDetail::ComputeEntitiesBoxesAndWorldExtents(scene, entitiesBoxes);
    ChunkGrid chunkGrid(worldBounds);
    currentHeader.worldBounds = worldBounds;
    
    file->Write(&currentHeader, sizeof(Header));
    LevelAssetLoaderDetail::WriteChunkTable(file, chunkGrid, entitiesBoxes);
    LevelAssetLoaderDetail::WriteEntities(file, entitiesBoxes);*/

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
    return Vector<const Type*>{ Type::Instance<FullLevelKey>(), Type::Instance<StreamLevelKey>(), Type::Instance<StreamEntityKey>() };
}

Vector<String> LevelAssetLoader::GetDependsOnFiles(const AssetBase* asset) const
{
    const Any& assetKey = asset->GetKey();
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
    DVASSERT(0 && "Wrong asset");
    return Vector<String>{};
}

template <>
bool AnyCompare<LevelAssetLoader::FullLevelKey>::IsEqual(const Any& v1, const Any& v2)
{
    return v1.Get<LevelAssetLoader::FullLevelKey>().path == v2.Get<LevelAssetLoader::FullLevelKey>().path;
}

template <>
bool AnyCompare<LevelAssetLoader::StreamLevelKey>::IsEqual(const Any& v1, const Any& v2)
{
    return v1.Get<LevelAssetLoader::StreamLevelKey>().path == v2.Get<LevelAssetLoader::StreamLevelKey>().path;
}

} // namespace DAVA
