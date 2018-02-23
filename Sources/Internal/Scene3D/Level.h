#pragma once
#include "Asset/Asset.h"
#include "Asset/AssetBase.h"
#include "Asset/AssetManager.h"
#include "Math/AABBox3.h"
#include "Scene3D/SceneFile/SerializationContext.h"
#include "Logger/Logger.h"

namespace DAVA
{
class DataNode;
class Scene;
class Camera;
class Level : public AssetBase
{
public:
    using StreamingFunction = Function<void()>;
    struct StreamingEvent
    {
        enum eEvent
        {
            INVALID_EVENT = 0,
            ENTITY_ADDED,
            ENTITY_REMOVED,
        };
        eEvent event = INVALID_EVENT;
        Entity* entity = nullptr;
    };

    void Setup(float32 solidAngleOfObjectsToLoad, const Vector3& startLoadPosition, float32 fullLoadRadius);
    void Load(const FilePath& filepath) override;
    void Save(const FilePath& filepath) override;
    void Reload() override;

    void StartStreaming(StreamingFunction streamingCallback);
    const Vector<StreamingEvent>& GetActiveStreamingEvents() const;
    const Vector<Entity*>& GetActiveEntities() const;
    void UpdateCamera(Camera* camera);
    void StopStreaming();

    void SetScene(Scene* scene_);

    /*
     TODO: Think about saving only part of .level
     */
    void MarkEntityForSave(Entity* entity);

private:
    /*
		Targets: up to 4 millions of objects. 

		File structure
		[Header]
		[DataNodeInfoTable]
		[ChunkInfoTable]
		[Entities]
	*/
    struct Header
    {
        enum
        {
            CURRENT_VERSION = 1,
        };

        char signature[4] = { 'L', 'V', 'L', 'B' };
        uint32 version = CURRENT_VERSION;
        uint32 allEntitiesCount = 0; // number of entities in whole map
    };

    struct DataNodeInfo
    {
        FilePath externalNodeFilepath;
    };

    /*	
		Estimation for table size is 6MB per 500.000 entities.
		Average objects count - 80~100 objects per block 100x100 meters. 
		8km x 8km = 80 x 80 chunks * 4bytes * 128 indices = 3.2MB. 
		During development Chunks can have place up to 256 objects each, and during export shrink indices. 
	*/

    struct Chunk
    {
        uint32 entitiesCount;
        Vector<uint32> entitiesIndices;
    };

    struct ChunkCoord
    {
        ChunkCoord() = default;
        ChunkCoord(int32 x_, int32 y_)
            : x(x_)
            , y(y_){};
        int32 x = 0;
        int32 y = 0;
    };

    struct ChunkBounds
    {
        ChunkCoord min;
        ChunkCoord max;
    };

    struct EntityInfo
    {
        uint32 fileOffset; // maybe uint64 --> see File
        uint32 fileSize;
        int16 boundMinX;
        int16 boundMinY;
        int16 boundMinZ;
        int16 boundMaxX;
        int16 boundMaxY;
        int16 boundMaxZ;
    };

    struct ChunkGrid
    {
        const float32 chunkSize = 100.0f; // 100x100 square meters
        ChunkGrid(const AABBox3& worldBounds);

        Chunk* GetChunk(const ChunkCoord& coord);
        ChunkBounds ProjectBoxOnGrid(const AABBox3& entityBox);

        ChunkBounds worldChunkBounds;
        uint32 chunkXCount = 0;
        uint32 chunkYCount = 0;
        AABBox3 worldBounds;
        Vector<Chunk> chunkData;
    };
    /* Functions for serialization */
    //void
    AABBox3 ComputeEntitiesBoxesAndWorldExtents(Vector<AABBox3>& boxes);
    void WriteChunkTable(File* file, ChunkGrid& chunkGrid, const Vector<AABBox3>& entitiesBoxes);
    void WriteEntities(File* file, const Vector<AABBox3>& entitiesBoxes);
    void SaveHierarchy(File* file, Entity* node, KeyedArchive* keyedArchive, int32 level);
    void WriteDataNodes();
    void WriteGeometries();

    Map<DataNode*, FilePath> dataNodesWithNames;
    Map<DataNode*, Set<FastName>> ownerSet;

    /* Functions for streaming */
    //    void RequestDataNode(uint32 dataNodeIndex);
    //    void DataNodeLoadedCallback(std::pair<uint32, DataNode*> dataNode);
    //
    //    void RequestEntity(uint32 entityIndex);
    //    void EntityLoadedCallback(std::pair<uint32, Entity*> entity);
    Vector<StreamingEvent> activeStreamingEvents;
    Vector<Entity*> activeEntities;

    // Init params
    float32 solidAngleOfObjectsToLoad;
    Vector3 startLoadPosition;
    float32 fullLoadRadius;

    struct EntityRuntimeInfo
    {
        uint32 lodLevel; // ? level of
    };

    Vector3 lastCameraPosition;
    Camera* camera = nullptr;
    UnorderedMap<Entity*, EntityRuntimeInfo> loadedEntities;

    /*
		Counters 
	*/
    // Average Data Node Load Speed
    // Average Entity Size On Disk
    // Average Entity Load Speed In Ms
    Scene* scene = nullptr;
    bool isDebugLogEnabled = false;
    SerializationContext serializationContext;
};

LoggerEnable(Level);
};
