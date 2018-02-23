#include "Scene3D/Level.h"
#include "Scene3D/Scene.h"
#include "FileSystem/File.h"
#include "FileSystem/FileSystem.h"
#include "Base/BaseMath.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Render/3D/PolygonGroup.h"
#include "Render/Material/NMaterial.h"
#include "Scene3D/AnimationData.h"

namespace DAVA
{
void Level::SetScene(Scene* scene_)
{
    scene = scene_;
}

void Level::Setup(float32 solidAngleOfObjectsToLoad, const Vector3& position, float32 fullLoadRadius)
{
}

void Level::Load(const FilePath& filepath)
{
    //
}

void Level::Save(const FilePath& filepath)
{
    DVASSERT(scene != nullptr);

    bool createFromScratch = true;
    File* _file = nullptr;
    if (createFromScratch)
    {
        _file = File::Create(filepath, File::CREATE | File::WRITE);
    }
    else
    {
        _file = File::Create(filepath, File::OPEN | File::WRITE);
        createFromScratch = true;
    }

    ScopedPtr<File> file(_file);
    if (!file)
    {
        Logger::Error("Level::Save failed to create file: %s", filepath.GetAbsolutePathname().c_str());
        //SetError(ERROR_FAILED_TO_CREATE_FILE);
        return; // ERROR_FAILED_TO_CREATE_FILE;
    }

    if (createFromScratch)
    {
        Header currentHeader;
        currentHeader.allEntitiesCount = scene->GetChildrenCount();

        Vector<AABBox3> entitiesBoxes;
        AABBox3 worldBounds = ComputeEntitiesBoxesAndWorldExtents(entitiesBoxes);
        ChunkGrid chunkGrid(worldBounds);

        WriteDataNodes();

        file->Write(&currentHeader, sizeof(Header));
        WriteChunkTable(file, chunkGrid, entitiesBoxes);
        WriteEntities(file, entitiesBoxes);
    }
    else
    {
        Header previousHeader;
        file->Read(&previousHeader, sizeof(Header));
        file->Seek(0, File::SEEK_FROM_START);

        //UpdateChunkTable();
        //UpdateEntities();
    }
}

Level::ChunkGrid::ChunkGrid(const AABBox3& worldBounds_)
    : worldBounds(worldBounds_)
{
    Vector3 size = worldBounds.GetSize();

    worldChunkBounds = ProjectBoxOnGrid(worldBounds);

    chunkXCount = worldChunkBounds.max.x - worldChunkBounds.min.x + 1;
    chunkYCount = worldChunkBounds.max.y - worldChunkBounds.min.y + 1;

    chunkData.resize(chunkXCount * chunkYCount);
}

Level::ChunkBounds Level::ChunkGrid::ProjectBoxOnGrid(const AABBox3& entityBox)
{
    ChunkBounds bounds;

    bounds.min.x = (int32)(entityBox.min.x / chunkSize);
    bounds.min.y = (int32)(entityBox.min.y / chunkSize);

    bounds.max.x = (int32)(entityBox.max.x / chunkSize);
    bounds.max.y = (int32)(entityBox.max.y / chunkSize);

    return bounds;
}

Level::Chunk* Level::ChunkGrid::GetChunk(const ChunkCoord& coord)
{
    uint32 address = (coord.y - worldChunkBounds.min.y) * chunkXCount +
    (coord.x - worldChunkBounds.min.x);
    if (address < chunkData.size())
        return &chunkData[address];
    return nullptr;
}

AABBox3 Level::ComputeEntitiesBoxesAndWorldExtents(Vector<AABBox3>& boxes)
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

void Level::WriteChunkTable(File* file,
                            ChunkGrid& chunkGrid,
                            const Vector<AABBox3>& entitiesBoxes)
{
    file->Write(&chunkGrid.worldChunkBounds, sizeof(chunkGrid.worldChunkBounds));
    file->Write(&chunkGrid.chunkXCount, sizeof(chunkGrid.chunkXCount));
    file->Write(&chunkGrid.chunkYCount, sizeof(chunkGrid.chunkYCount));
    file->Write(&chunkGrid.worldBounds, sizeof(chunkGrid.worldBounds));

    // Populate chunks with data
    size_t maxEntitiesInChunk = 0;

    size_t entityCount = entitiesBoxes.size();
    for (size_t entityIndex = 0; entityIndex < entityCount; ++entityIndex)
    {
        ChunkBounds bounds = chunkGrid.ProjectBoxOnGrid(entitiesBoxes[entityIndex]);

        for (int32 y = bounds.min.y; y <= bounds.max.y; ++y)
            for (int32 x = bounds.min.x; x <= bounds.max.x; ++x)
            {
                Chunk* chunk = chunkGrid.GetChunk(ChunkCoord(x, y));
                chunk->entitiesIndices.emplace_back(static_cast<uint32>(entityIndex));

                maxEntitiesInChunk = Max(maxEntitiesInChunk, chunk->entitiesIndices.size());
            }
        Logger::Debug<Level>("bounds: %d, %d, %d, %d",
                             bounds.min.x, bounds.min.y,
                             bounds.max.x, bounds.max.y);
    }
    Logger::Debug<Level>("max: %d", maxEntitiesInChunk);

    file->Write(&maxEntitiesInChunk, sizeof(maxEntitiesInChunk));

    // Write chunks to disk
    for (int32 y = chunkGrid.worldChunkBounds.min.y; y <= chunkGrid.worldChunkBounds.max.y; ++y)
        for (int32 x = chunkGrid.worldChunkBounds.min.x; x <= chunkGrid.worldChunkBounds.max.x; ++x)
        {
            Chunk* chunk = chunkGrid.GetChunk(ChunkCoord(x, y));
            Logger::Debug<Level>("objs: %d, %d, %d", x, y, chunk->entitiesIndices.size());
            DVASSERT(chunk != nullptr);
            DVASSERT(chunk->entitiesIndices.size() <= maxEntitiesInChunk);
            uint32* data = chunk->entitiesIndices.data();
            uint16 cnt = static_cast<uint16>(chunk->entitiesIndices.size());
            file->Write(&cnt, sizeof(uint16));
            if (cnt > 0)
                file->Write(data, sizeof(uint32) * cnt);
        }
}

void Level::WriteEntities(File* file, const Vector<AABBox3>& entitiesBoxes)
{
    DVASSERT(sizeof(EntityInfo) == 20);

    uint32 entityCount = static_cast<uint32>(entitiesBoxes.size());
    uint64 entityInfoTableStartPos = file->GetPos();
    uint32 entityInfoTableSize = sizeof(EntityInfo) * entityCount;

    // Pass the table
    file->Seek(entityInfoTableSize, File::SEEK_FROM_CURRENT);

    Vector<EntityInfo> infoArray(entityCount);
    // Save entities and record entity info offsets

    auto GetOwnerSet = [&](DataNode* node) -> String
    {
        String res = "";
        for (const FastName& owner : ownerSet[node])
        {
            res += String(owner.c_str());
        }
        return res;
    };

    for (int32 entityIndex = 0; entityIndex < static_cast<int32>(entityCount); ++entityIndex)
    {
        Entity* entity = scene->GetChild(entityIndex);
        KeyedArchive* archive = new KeyedArchive();
        SaveHierarchy(file, entity, archive, 0);

        EntityInfo& entityInfo = infoArray[entityIndex];
        entityInfo.fileOffset = static_cast<uint32>(file->GetPos());
        archive->Save(file);
        entityInfo.fileSize = static_cast<uint32>(file->GetPos() - entityInfo.fileOffset);

        SafeRelease(archive);
        /** Debug info **/
        Set<DataNode*> dataNodesInEntity;
        entity->GetDataNodes(dataNodesInEntity);
        Logger::Debug<Level>("Entity: %s DataNodes Count: %d", entity->GetName().c_str(), dataNodesInEntity.size());
        for (DataNode* node : dataNodesInEntity)
        {
            PolygonGroup* geometry = dynamic_cast<PolygonGroup*>(node);
            if (geometry)
            {
                Logger::Debug<Level>("-- Geometry: %s vertices: %d, primitives: %d owners: %s", dataNodesWithNames[node].GetFilename().c_str(),
                                     geometry->GetVertexCount(), geometry->GetPrimitiveCount(), GetOwnerSet(node).c_str());
            }
            else
                Logger::Debug<Level>("-- DataNode: %s owners: %s", dataNodesWithNames[node].GetFilename().c_str(), GetOwnerSet(node).c_str());
        }
    }

    // Build entity info table
    for (size_t entityIndex = 0; entityIndex < entityCount; ++entityIndex)
    {
        EntityInfo& entityInfo = infoArray[entityIndex];

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

void Level::SaveHierarchy(File* file, Entity* entity, KeyedArchive* keyedArchive, int32 level)
{
    if (isDebugLogEnabled)
    {
        Logger::FrameworkDebug("%s %s(%s) %d", GetIndentString('-', level).c_str(), entity->GetName().c_str(), entity->GetClassName().c_str(), entity->GetChildrenCount());
    }
    entity->Save(keyedArchive, &serializationContext);
    keyedArchive->SetInt32("#childrenCount", entity->GetChildrenCount());

    for (int ci = 0; ci < entity->GetChildrenCount(); ++ci)
    {
        KeyedArchive* childArchive = new KeyedArchive();
        Entity* child = entity->GetChild(ci);
        SaveHierarchy(file, child, childArchive, level + 1);
        keyedArchive->SetArchive(Format("#child_%d", ci), childArchive);
        SafeRelease(childArchive);
    }
}

void Level::WriteGeometries()
{
}

void Level::WriteDataNodes()
{
    int32 entityCount = static_cast<int32>(scene->GetChildrenCount());
    for (int32 entityIndex = 0; entityIndex < entityCount; ++entityIndex)
    {
        Entity* entity = scene->GetChild(entityIndex);
        Set<DataNode*> dataNodesInEntity;
        entity->GetDataNodes(dataNodesInEntity);
        for (DataNode* node : dataNodesInEntity)
        {
            ownerSet[node].insert(entity->GetName());
        }
    }

    Set<DataNode*> datanodes;
    scene->GetDataNodes(datanodes);

    Map<String, int32> nodeIndex
    {
      { "mat", 0 },
      { "geo", 0 },
      { "anim", 0 },
      { "unknown", 0 },
    };

    FilePath dataNodeFilepath = GetFilepath();
    FilePath levelDirectory = dataNodeFilepath.GetDirectory();
    for (auto dir : nodeIndex)
    {
        FileSystem::Instance()->CreateDirectory(levelDirectory + dir.first);
    }

    for (DataNode* dataNode : datanodes)
    {
        String extension = "unknown";

        NMaterial* material = dynamic_cast<NMaterial*>(dataNode);
        if (material)
        {
            extension = "mat";
        }
        PolygonGroup* geometry = dynamic_cast<PolygonGroup*>(dataNode);
        if (geometry)
        {
            extension = "geo";
        }
        AnimationData* animationData = dynamic_cast<AnimationData*>(dataNode);
        if (animationData)
        {
            extension = "anim";
        }
        /*
        SettingsNode* settingsNode = dynamic_cast<SettingsNode*>(dataNode);
        if (settingsNode)
        {
            extension = "set";
        }*/

        FilePath finalFilePath = levelDirectory + Format("%s/datanode_%03d.%s", extension.c_str(), nodeIndex[extension], extension.c_str());

        KeyedArchive* archive = new KeyedArchive();
        dataNode->Save(archive, &serializationContext);
        archive->Save(finalFilePath);
        SafeRelease(archive);
        dataNodesWithNames.emplace(dataNode, finalFilePath);

        nodeIndex[extension]++;
    }

    //
    // Logger::Debug<Level>("Geometry nodes reuse between models:");
    uint32 geoReuse = 0;
    for (DataNode* dataNode : datanodes)
    {
        PolygonGroup* geometry = dynamic_cast<PolygonGroup*>(dataNode);
        if (geometry)
        {
            if (ownerSet[dataNode].size() != 1)
            {
                Logger::Debug<Level>("Geo reuse: %s cnt: %d", dataNodesWithNames[dataNode].GetAbsolutePathname().c_str(), ownerSet[dataNode].size());
                geoReuse++;
            }
        }
    }
    DVASSERT(geoReuse == 0);
}

void Level::Reload()
{
}

void Level::StartStreaming(StreamingFunction streamingCallback)
{
}

const Vector<Level::StreamingEvent>& Level::GetActiveStreamingEvents() const
{
    return activeStreamingEvents;
}

const Vector<Entity*>& Level::GetActiveEntities() const
{
    return activeEntities;
}

void Level::UpdateCamera(Camera* camera_)
{
    if (camera == nullptr)
    {
        camera = camera_;
    }
}

void Level::StopStreaming()
{
}

void Level::MarkEntityForSave(Entity* entity)
{
}
};
