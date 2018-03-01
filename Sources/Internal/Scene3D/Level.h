#pragma once

#include "Asset/Asset.h"
#include "Asset/AssetManager.h"
#include "Math/AABBox3.h"
#include "Scene3D/SceneFile/SerializationContext.h"
#include "Logger/Logger.h"
#include "Concurrency/AutoResetEvent.h"
#include <mutex>

namespace DAVA
{
class DataNode;
class Scene;
class Camera;
//class Level : public AssetBase
//{
//public:
//    Level();
//    ~Level();
//
//    using StreamingFunction = Function<void()>;
//    struct StreamingEvent
//    {
//        enum eEventType
//        {
//            INVALID_EVENT = 0,
//            ENTITY_ADDED,
//            ENTITY_REMOVED,
//        };
//        eEventType type = INVALID_EVENT;
//        Entity* entity = nullptr;
//    };
//
//    void Load(const FilePath& filepath) override;
//    void Save(const FilePath& filepath) override;
//    void Reload() override;
//
//    void Setup(float32 solidAngleOfObjectsToLoad, const Vector3& startLoadPosition, float32 fullLoadRadius);
//    void StartStreaming(StreamingFunction streamingCallback);
//    const Vector<StreamingEvent>& GetActiveStreamingEvents() const;
//    void ClearActiveStreamingEvents();
//    const Vector<Entity*>& GetActiveEntities() const;
//    void ProcessStreaming(Camera* camera);
//    void StopStreaming();
//
//    void SetScene(Scene* scene_);
//
//    /*
//     TODO: Think about saving only part of .level
//     */
//    void MarkEntityForSave(Entity* entity);
//
//private:
//    /*
//		Targets: up to 4 millions of objects.
//
//		File structure
//		[Header]
//		[DataNodeInfoTable]
//		[ChunkInfoTable]
//		[Entities]
//	*/
//    struct Header
//    {
//        enum
//        {
//            CURRENT_VERSION = 2,
//            SUPPORTED_VERSION = 2,
//        };
//
//        char8 signature[4] = { 'L', 'V', 'L', 'B' };
//        uint32 version = CURRENT_VERSION;
//        uint32 allEntitiesCount = 0; // number of entities in whole map
//        AABBox3 worldBounds;
//    };
//
//    struct DataNodeInfo
//    {
//        FilePath externalNodeFilepath;
//    };
//
//    /*
//		Estimation for table size is 6MB per 500.000 entities.
//		Average objects count - 80~100 objects per block 100x100 meters.
//		8km x 8km = 80 x 80 chunks * 4bytes * 128 indices = 3.2MB.
//		During development Chunks can have place up to 256 objects each, and during export shrink indices.
//	*/
//
//    struct Chunk
//    {
//        enum eState
//        {
//            STATE_NOT_REQUESTED,
//            STATE_REQUESTED,
//        };
//
//        uint32 entitiesCount = 0;
//        Vector<uint32> entitiesIndices;
//
//        uint32 state = STATE_NOT_REQUESTED;
//
//        uint32 visitedLastFrameIndex = 0;
//    };
//
//    struct ChunkCoord
//    {
//        ChunkCoord() = default;
//        ChunkCoord(int32 x_, int32 y_)
//            : x(x_)
//            , y(y_){};
//
//        bool operator==(const ChunkCoord& other)
//        {
//            return x == other.x && y == other.y;
//        }
//        bool operator!=(const ChunkCoord& other)
//        {
//            return x != other.x || y != other.y;
//        }
//
//        int32 x = 0;
//        int32 y = 0;
//    };
//
//    struct ChunkBounds
//    {
//        ChunkCoord min;
//        ChunkCoord max;
//    };
//
//    struct EntityInfo
//    {
//        uint32 fileOffset;
//        uint32 fileSize;
//        int16 boundMinX;
//        int16 boundMinY;
//        int16 boundMinZ;
//        int16 boundMaxX;
//        int16 boundMaxY;
//        int16 boundMaxZ;
//    };
//
//    struct ChunkGrid
//    {
//        const float32 chunkSize = 100.0f; // 100x100 square meters
//        ChunkGrid(const AABBox3& worldBounds);
//
//        uint32 GetChunkAddress(const ChunkCoord& coord);
//        Chunk* GetChunk(uint32 address);
//        Chunk* GetChunk(const ChunkCoord& coord);
//        ChunkBounds ProjectBoxOnGrid(const AABBox3& entityBox);
//        ChunkCoord GetChunkCoord(const Vector3& position);
//
//        ChunkBounds worldChunkBounds;
//        uint32 chunkXCount = 0;
//        uint32 chunkYCount = 0;
//        AABBox3 worldBounds;
//        Chunk specialStreamingSettingsChunk;
//        Vector<Chunk> chunkData;
//    };
//
//    struct ChunkStreamingInfo
//    {
//        ChunkStreamingInfo()
//        {
//            distanceFromCenter = 0;
//        }
//
//        uint32 distanceFromCenter : 7;
//    };
//
//    struct StreamingLoadRequest
//    {
//        int32 priority = 0;
//        uint32 entityIndex = 0;
//        uint32 chunkAddress = 0;
//        bool operator<(const StreamingLoadRequest& other) const
//        {
//            return priority > other.priority;
//        }
//    };
//
//    /* Functions for serialization */
//    //void
//    AABBox3 ComputeEntitiesBoxesAndWorldExtents(Vector<AABBox3>& boxes);
//    void WriteChunkTable(File* file, ChunkGrid& chunkGrid, const Vector<AABBox3>& entitiesBoxes);
//    void WriteEntities(File* file, const Vector<AABBox3>& entitiesBoxes);
//    void SaveHierarchy(File* file, Entity* node, KeyedArchive* keyedArchive, int32 level);
//    void WriteDataNodes();
//    void WriteGeometries();
//
//    Map<DataNode*, FilePath> dataNodesWithNames;
//    Map<DataNode*, Set<FastName>> ownerSet;
//
//    /* Functions for streaming */
//    //    void RequestDataNode(uint32 dataNodeIndex);
//    //    void DataNodeLoadedCallback(std::pair<uint32, DataNode*> dataNode);
//    //
//    //    void RequestEntity(uint32 entityIndex);
//    //    void EntityLoadedCallback(std::pair<uint32, Entity*> entity);
//
//    void ReadChunkTable(File* file, ChunkGrid& chunkGrid);
//    void ReadAllEntities(File* file, uint32 entityCount);
//
//    void RequestGlobalChunk();
//    void RequestLoadChunk(const ChunkCoord& chunkCoord);
//    void RequestUnloadChunk(const ChunkCoord& chunkCoord);
//    void RequestLoadEntity(uint32 entityIndex, uint32 chunkAddress);
//    void RequestUnloadEntity(uint32 entityIndex, uint32 chunkAddress);
//    void StreamingThread();
//
//    Thread* streamingThread = nullptr;
//    File* streamingFile = nullptr;
//    ChunkCoord previousCameraPos = { -1000, -1000 };
//    ChunkGrid* loadedChunkGrid = nullptr;
//    Vector<ChunkStreamingInfo> chunkStreamingInfo;
//    Vector<EntityInfo> loadedInfoArray;
//
//    struct EntityWithRefCounter
//    {
//        enum eState : uint32
//        {
//            STATE_EMPTY = 0,
//            STATE_QUEUED,
//            STATE_LOADING,
//            STATE_LOADED,
//            STATE_LOADING_CANCEL,
//        };
//
//        eState state = STATE_EMPTY;
//        Entity* entity = nullptr;
//        uint32 counter = 0;
//    };
//
//    Vector<EntityWithRefCounter> loadedEntityArray;
//    Vector<uint32> pendingDelete;
//
//    AutoResetEvent requestsAvailable;
//    std::deque<StreamingLoadRequest> loadRequestQueue;
//    Mutex requestsMutex;
//
//    Vector<StreamingEvent> activeStreamingEvents;
//    Mutex eventsMutex;
//
//    // Init params
//    float32 solidAngleOfObjectsToLoad;
//    Vector3 startLoadPosition;
//    float32 fullLoadRadius;
//
//    struct EntityRuntimeInfo
//    {
//        uint32 lodLevel; // ? level of
//    };
//
//    Vector3 cameraPosition;
//    Vector3 movementDirection;
//    UnorderedMap<Entity*, EntityRuntimeInfo> loadedEntities;
//
//    /*
//		Counters
//	*/
//    // Average Data Node Load Speed
//    // Average Entity Size On Disk
//    // Average Entity Load Speed In Ms
//    Scene* scene = nullptr;
//    bool isDebugLogEnabled = false;
//    SerializationContext serializationContext;
//    StreamingFunction streamingCallback;
//
//    // Dump
//    DAVA::Thread* dumpThread = nullptr;
//    std::mutex lock;
//    std::condition_variable notifier;
//    bool hasProducedMetrics = false;
//};
//
//LoggerEnable(Level);
};
