#include "Scene3D/Level.h"
#include "Scene3D/Scene.h"
#include "FileSystem/File.h"
#include "FileSystem/FileSystem.h"
#include "Base/BaseMath.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Components/StreamingSettingsComponent.h"
#include "Render/3D/PolygonGroup.h"
#include "Render/Material/NMaterial.h"
#include "Scene3D/AnimationData.h"
#include "Scene3D/SceneSerialization.h"
#include "Engine/Engine.h"
#include "Job/JobManager.h"
#include "Debug/ProfilerMarkerNames.h"
#include "Debug/ProfilerCPU.h"
#include "Debug/ProfilerGPU.h"
#include "Time/SystemTimer.h"

#define DISABLE_LEVEL_STREAMING 1

namespace DAVA
{
Level::Level()
{
    serializationContext.SetVersion(STREAMING_SCENE_VERSION);
}

Level::~Level()
{
    StopStreaming();

    SafeDelete(loadedChunkGrid);
    SafeRelease(streamingFile);
}

void Level::SetScene(Scene* scene_)
{
    scene = scene_;
}

void Level::Load(const FilePath& filepath)
{
    //
    streamingFile = File::Create(filepath, File::OPEN | File::READ);
    if (!streamingFile)
    {
        return;
    }
    Header currentHeader;
    streamingFile->Read(&currentHeader, sizeof(Header));

    if (currentHeader.version < Header::SUPPORTED_VERSION)
    {
        DAVA_THROW(Exception, "Unsupported version of .level file.");
    }

    serializationContext.SetScenePath(filepath.GetDirectory());

    loadedChunkGrid = new ChunkGrid(currentHeader.worldBounds);
    ReadChunkTable(streamingFile, *loadedChunkGrid);
    ReadAllEntities(streamingFile, currentHeader.allEntitiesCount);
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

    serializationContext.SetScenePath(filepath.GetDirectory());

    if (createFromScratch)
    {
        Header currentHeader;
        currentHeader.allEntitiesCount = scene->GetChildrenCount();

        Vector<AABBox3> entitiesBoxes;
        AABBox3 worldBounds = ComputeEntitiesBoxesAndWorldExtents(entitiesBoxes);
        ChunkGrid chunkGrid(worldBounds);
        currentHeader.worldBounds = worldBounds;

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

Level::ChunkCoord Level::ChunkGrid::GetChunkCoord(const Vector3& position)
{
    ChunkCoord coord;
    coord.x = (int32)(position.x / chunkSize);
    coord.y = (int32)(position.y / chunkSize);
    return coord;
}

uint32 Level::ChunkGrid::GetChunkAddress(const ChunkCoord& coord)
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
            ChunkBounds bounds = chunkGrid.ProjectBoxOnGrid(entitiesBoxes[entityIndex]);

            for (int32 y = bounds.min.y; y <= bounds.max.y; ++y)
                for (int32 x = bounds.min.x; x <= bounds.max.x; ++x)
                {
                    Chunk* chunk = chunkGrid.GetChunk(ChunkCoord(x, y));
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
        Chunk* chunk = &chunkGrid.specialStreamingSettingsChunk;
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
            Chunk* chunk = chunkGrid.GetChunk(ChunkCoord(x, y));
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

void Level::ReadChunkTable(File* file,
                           ChunkGrid& chunkGrid)
{
    ChunkBounds worldChunkBounds;
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
        Chunk* chunk = &chunkGrid.specialStreamingSettingsChunk;
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
            Chunk* chunk = chunkGrid.GetChunk(ChunkCoord(x, y));
            uint16 cnt = 0;
            file->Read(&cnt, sizeof(uint16));
            chunk->entitiesIndices.resize(cnt);
            if (cnt > 0)
            {
                file->Read(chunk->entitiesIndices.data(), sizeof(uint32) * cnt);
            }
        }

    chunkStreamingInfo.resize(chunkGrid.chunkXCount * chunkGrid.chunkYCount);
}

void Level::WriteEntities(File* file, const Vector<AABBox3>& entitiesBoxes)
{
    DVASSERT(sizeof(EntityInfo) == 20);

    size_t entityCount = entitiesBoxes.size();
    uint64 entityInfoTableStartPos = file->GetPos();
    uint32 entityInfoTableSize = uint32(sizeof(EntityInfo) * entityCount);

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

    for (size_t entityIndex = 0; entityIndex < entityCount; ++entityIndex)
    {
        Entity* entity = scene->GetChild(int32(entityIndex));
        KeyedArchive* archive = new KeyedArchive();
        SceneSerialization::SaveHierarchy(entity, archive, &serializationContext, SceneSerialization::LEVEL);

        EntityInfo& entityInfo = infoArray[entityIndex];
        entityInfo.fileOffset = uint32(file->GetPos());
        archive->Save(file);
        entityInfo.fileSize = uint32(file->GetPos()) - entityInfo.fileOffset;

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

void Level::ReadAllEntities(File* file, uint32 entityCount)
{
    DVASSERT(sizeof(EntityInfo) == 20);

    uint32 entityInfoTableSize = sizeof(EntityInfo) * entityCount;
    loadedInfoArray.resize(entityCount);
    loadedEntityArray.resize(entityCount);
    file->Read(loadedInfoArray.data(), entityInfoTableSize);

#if DISABLE_LEVEL_STREAMING
    for (size_t entityIndex = 0; entityIndex < entityCount; ++entityIndex)
    {
        EntityInfo& entityInfo = loadedInfoArray[entityIndex];

        file->Seek(entityInfo.fileOffset, File::SEEK_FROM_START);
        KeyedArchive* archive = new KeyedArchive();
        archive->Load(file);

        Entity* entity = SceneSerialization::LoadHierarchy(nullptr, archive, &serializationContext, SceneSerialization::LEVEL);
        activeStreamingEvents.push_back({ StreamingEvent::ENTITY_ADDED, entity });

        SafeRelease(archive);
    }
#endif
}

void Level::WriteGeometries()
{
}

void Level::WriteDataNodes()
{
    size_t entityCount = scene->GetChildrenCount();
    for (size_t entityIndex = 0; entityIndex < entityCount; ++entityIndex)
    {
        Entity* entity = scene->GetChild(int32(entityIndex));
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

void Level::StartStreaming(StreamingFunction streamingCallback_)
{
    streamingCallback = streamingCallback_;

#if !DISABLE_LEVEL_STREAMING
    DAVA::Function<void()> streamingThreadFunc = DAVA::MakeFunction(this, &Level::StreamingThread);
    streamingThread = DAVA::Thread::Create(streamingThreadFunc);
    streamingThread->SetName("Streaming Thread");
    streamingThread->Start();
    streamingThread->SetPriority(Thread::PRIORITY_NORMAL);

    dumpThread = Thread::Create([this]
                                {
                                    Thread* thread = Thread::Current();
                                    do
                                    {
                                        std::unique_lock<std::mutex> guard(lock);
                                        notifier.wait(guard, [this]() { return hasProducedMetrics; });
                                        TraceEvent::AppendJSON(ProfilerCPU::globalProfiler->GetTrace(0), "~doc:/profile.json");
                                        hasProducedMetrics = false;
                                    } while (!thread->IsCancelling());
                                });
    dumpThread->SetName("ProfileDumpThread");
    dumpThread->Start();

    RequestGlobalChunk();
#endif
}

const Vector<Level::StreamingEvent>& Level::GetActiveStreamingEvents() const
{
    return activeStreamingEvents;
}

void Level::StopStreaming()
{
    if (streamingThread)
    {
        streamingThread->Cancel();
        requestsAvailable.Signal();

        if (streamingThread->IsJoinable())
        {
            streamingThread->Join();
        }
        streamingThread->Release();
        streamingThread = nullptr;
    }
    streamingCallback = nullptr;
}

void Level::StreamingThread()
{
    Thread* thread = Thread::Current();
    do
    {
        requestsAvailable.Wait();
        if (thread->IsCancelling())
            break;

        // TODO: Mutex ?
        while (loadRequestQueue.size() != 0)
        {
            if (thread->IsCancelling())
                break;

            requestsMutex.Lock();
            StreamingLoadRequest loadRequest = loadRequestQueue.front();
            loadRequestQueue.pop_front();

            Logger::Debug("Start entity request from: %d queue: %d",
                          loadRequest.entityIndex, loadRequestQueue.size());

            EntityWithRefCounter& entityWithCounter = loadedEntityArray[loadRequest.entityIndex];
            entityWithCounter.state = EntityWithRefCounter::STATE_LOADING;
            requestsMutex.Unlock();

            DVASSERT(loadRequest.entityIndex < loadedInfoArray.size());

            KeyedArchive* archive = nullptr;
            Entity* entity = nullptr;
            EntityInfo& entityInfo = loadedInfoArray[loadRequest.entityIndex];
            {
                Logger::Debug("Read entity from file: %d", loadRequest.entityIndex);

                DAVA_PROFILER_CPU_SCOPE(ProfilerCPUMarkerName::ASSET_STREAMING_THREAD_READ_ROOT_ENTITY);
                streamingFile->Seek(entityInfo.fileOffset, File::SEEK_FROM_START);
                archive = new KeyedArchive();
                archive->Load(streamingFile);
            }
            {
                Logger::Debug("Create entity from file: %d", loadRequest.entityIndex);
                DAVA_PROFILER_CPU_SCOPE(ProfilerCPUMarkerName::ASSET_STREAMING_THREAD_SCENE_DESERIALIZE);
                entity = SceneSerialization::LoadHierarchy(nullptr, archive, &serializationContext, SceneSerialization::LEVEL);
                SafeRelease(archive);
                Logger::Debug("Create finish from file: %d", loadRequest.entityIndex);
            }

            requestsMutex.Lock();
            entityWithCounter.entity = entity;
            entityWithCounter.state = EntityWithRefCounter::STATE_LOADED;
            requestsMutex.Unlock();

            auto addEventJob = [this, entity]()
            {
                activeStreamingEvents.push_back({ StreamingEvent::ENTITY_ADDED, entity });
            };
            GetEngineContext()->jobManager->CreateMainJob(addEventJob);
        }

    } while (!thread->IsCancelling());
}

void Level::RequestLoadEntity(uint32 entityIndex, uint32 chunkAddress)
{
    DVASSERT(entityIndex < loadedEntityArray.size());

    EntityWithRefCounter& entityWithCounter = loadedEntityArray[entityIndex];
    if (entityWithCounter.state == EntityWithRefCounter::STATE_EMPTY || entityWithCounter.state == EntityWithRefCounter::STATE_QUEUED)
    {
        EntityInfo& entityInfo = loadedInfoArray[entityIndex];
        int32 entitySize = (entityInfo.boundMaxX - entityInfo.boundMinX) * (entityInfo.boundMaxY - entityInfo.boundMinY) * (entityInfo.boundMaxZ - entityInfo.boundMinZ);

        entityWithCounter.counter++;

        requestsMutex.Lock();
        entityWithCounter.state = EntityWithRefCounter::STATE_QUEUED;
        loadRequestQueue.push_back({ entitySize, entityIndex, chunkAddress });
        std::sort(loadRequestQueue.begin(), loadRequestQueue.end());
        requestsMutex.Unlock();
        requestsAvailable.Signal();
    }
    else
    {
        entityWithCounter.counter++;
    }
}

void Level::RequestUnloadEntity(uint32 entityIndex, uint32 chunkAddress)
{
    DVASSERT(entityIndex < loadedEntityArray.size());
    EntityWithRefCounter& entityWithCounter = loadedEntityArray[entityIndex];
    if (entityWithCounter.counter == 0)
        return;

    entityWithCounter.counter--;
    if (entityWithCounter.counter == 0)
    {
        requestsMutex.Lock();
        if (entityWithCounter.state == EntityWithRefCounter::STATE_LOADING)
        {
            //entityWithCounter.state =
            pendingDelete.emplace_back(entityIndex);
            DVASSERT(0);
        }
        else if (entityWithCounter.state == EntityWithRefCounter::STATE_QUEUED)
        {
            bool erased = false;
            for (auto it = loadRequestQueue.begin(); it != loadRequestQueue.end(); ++it)
            {
                if ((it->entityIndex == entityIndex) && (it->chunkAddress == chunkAddress))
                {
                    erased = true;
                    loadRequestQueue.erase(it);
                    break;
                }
            }
            entityWithCounter.state = EntityWithRefCounter::STATE_EMPTY;
            DVASSERT(erased == true);
        }
        else
        {
            activeStreamingEvents.push_back({ StreamingEvent::ENTITY_REMOVED, entityWithCounter.entity });
            /*
             
             */
            //SafeRelease(entityWithCounter.entity);
            entityWithCounter.state = EntityWithRefCounter::STATE_EMPTY;
        }
        requestsMutex.Unlock();
    }
}

void Level::RequestGlobalChunk()
{
    Chunk* chunk = &loadedChunkGrid->specialStreamingSettingsChunk;
    for (uint32 entityIndex : chunk->entitiesIndices)
    {
        RequestLoadEntity(entityIndex, (uint32)(0xFFFFFFFF));
    }
}

void Level::RequestLoadChunk(const ChunkCoord& chunkCoord)
{
    uint32 chunkAddress = loadedChunkGrid->GetChunkAddress(chunkCoord);
    Chunk* chunk = loadedChunkGrid->GetChunk(chunkAddress);
    DVASSERT(chunk != nullptr);

    if (chunk->state == Chunk::STATE_NOT_REQUESTED)
    {
        Logger::Debug("Request chunk load: %d %d", chunkCoord.x, chunkCoord.y);

        chunk->state = Chunk::STATE_REQUESTED;
        for (uint32 entityIndex : chunk->entitiesIndices)
        {
            RequestLoadEntity(entityIndex, chunkAddress);
        }
    }
}

void Level::RequestUnloadChunk(const ChunkCoord& chunkCoord)
{
    uint32 chunkAddress = loadedChunkGrid->GetChunkAddress(chunkCoord);
    Chunk* chunk = loadedChunkGrid->GetChunk(chunkAddress);
    DVASSERT(chunk != nullptr);

    if (chunk->state == Chunk::STATE_REQUESTED)
    {
        Logger::Debug("Request chunk unload: %d %d", chunkCoord.x, chunkCoord.y);

        for (uint32 entityIndex : chunk->entitiesIndices)
        {
            RequestUnloadEntity(entityIndex, chunkAddress);
        }
        chunk->state = Chunk::STATE_NOT_REQUESTED;
    }
}

void Level::ProcessStreaming(Camera* camera)
{
    static const float32 dumpCycle = 0.5f;
    static float32 cycleTime = dumpCycle;
    cycleTime -= SystemTimer::GetFrameDelta();
    if (cycleTime < 0.f)
    {
        cycleTime = dumpCycle;
        std::unique_lock<std::mutex> guard(lock);
        //ProfilerCPU::globalProfiler->Stop();
        //ProfilerGPU::globalProfiler->Stop();
        //ProfilerCPU::globalProfiler->DeleteSnapshots();
        //ProfilerCPU::globalProfiler->MakeSnapshot();
        //ProfilerCPU::globalProfiler->Restart();
        //ProfilerGPU::globalProfiler->Start();
        hasProducedMetrics = true;
        notifier.notify_one();
    }

    if (camera)
    {
        movementDirection = camera->GetPosition() - cameraPosition;
        cameraPosition = camera->GetPosition();
    }

    ChunkCoord currentCameraPosInChunk = loadedChunkGrid->GetChunkCoord(cameraPosition);

    float32 visibilityRadiusMeters = 200.0f; // 1000 meters visibility
    int32 chunkVisibilityRadius = 5; //(visibilityRadiusMeters / loadedChunkGrid->chunkSize) / 2;

    if (previousCameraPos != currentCameraPosInChunk)
    {
        int32 shifts[8][2] = { { -1, 0 }, { 1, 0 }, { 0, -1 }, { 0, 1 }, { -1, -1 }, { -1, 1 }, { 1, -1 }, { 1, 1 } };

        std::queue<ChunkCoord> queue;
        queue.push(currentCameraPosInChunk);

        while (!queue.empty())
        {
            ChunkCoord currentPos = queue.front();
            queue.pop();

            if ((currentPos.x < loadedChunkGrid->worldChunkBounds.min.x)
                || (currentPos.x > loadedChunkGrid->worldChunkBounds.max.x)
                || (currentPos.y < loadedChunkGrid->worldChunkBounds.min.y)
                || (currentPos.y > loadedChunkGrid->worldChunkBounds.max.y))
            {
                // Exit from streaming update if we out of streaming area
                break;
            }

            ChunkCoord distance;
            distance.x = currentPos.x - currentCameraPosInChunk.x;
            distance.y = currentPos.y - currentCameraPosInChunk.y;
            int32 squareDistance = distance.x * distance.x + distance.y * distance.y;
            uint32 address = loadedChunkGrid->GetChunkAddress(currentPos);

            Chunk* chunk = loadedChunkGrid->GetChunk(address);
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
                    ChunkCoord newPos = currentPos;
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

    for (uint32 entityIndex : pendingDelete)
    {
        EntityWithRefCounter& entityWithCounter = loadedEntityArray[entityIndex];
        if (entityWithCounter.entity)
        {
            SafeRelease(entityWithCounter.entity);
            Logger::Debug("Pending delete: %d", entityIndex);
        }
    }
    pendingDelete.clear();

    if (streamingCallback)
    {
        streamingCallback();
    }
}

void Level::ClearActiveStreamingEvents()
{
    activeStreamingEvents.clear();
}

void Level::MarkEntityForSave(Entity* entity)
{
}
};
