#pragma once

#include "DataSection.hpp"
#include "Converter/Visitor/ConverterVisitorBase.h"
#include "unique_id.h"
#include "Model.h"
#include "ServerSPT.h"
#include "zip/contrib/minizip/zip.h"

#include <Scene3D/Components/Waypoint/PathComponent.h>
#include <FileSystem/FilePath.h>

#define BUFFERS_SZ 8192
#define TILEMASK_SZ 2048
#define DOM_RES 128

namespace DAVA
{
class Entity;
class Scene;
class Landscape;
class Heightmap;
class Image;
class KeyedArchive;
class EditorConfig;
class VersionInfo;
} // namespace DAVA

const DAVA::uint32 QUANTISED_PNG_VERSION = 0x71706e67; // 'qpng'
//const float	QUANTISATION_LEVEL	= 1000.0f;	// quantise heights to mm

struct PNGImageData
{
    DAVA::uint8* data_; // raw image data - this is never owned by PNGImageData
    DAVA::uint32 width_; // image width
    DAVA::uint32 height_; // image height
    DAVA::uint32 bpp_; // bits per pixel
    DAVA::uint32 stride_; // row stride in bytes
    bool upsideDown_; // is the image upside down in memory?

    PNGImageData();
};

struct HeightMapHeader
{
    enum HMHVersions
    {
        VERSION_ABS_FLOAT = 1, // absolute float values
        VERSION_REL_UINT16 = 2, // relative uint16 values
        VERSION_REL_UINT16_PNG = 3, // as above, compressed via png.
        VERSION_ABS_QFLOAT = 4 // absolute quantised float values
    };

    DAVA::uint32 magic_; // Should be "hmp\0"
    DAVA::uint32 width_;
    DAVA::uint32 height_;
    DAVA::uint32 compression_;
    DAVA::uint32 version_; // format version
    float minHeight_;
    float maxHeight_;
    DAVA::uint32 pad_;
    // static const uint32 MAGIC = '\0pmh';
    static const DAVA::uint32 MAGIC = 0x00706d68;
};

static const int RAW_COMPRESSION = 0;
static const int DEFAULT_COMPRESSION = 6;
static const int BEST_COMPRESSION = 10;
const DAVA::uint32 COMPRESSED_MAGIC1 = 0x7a697000; // "zip\0"
const DAVA::uint32 COMPRESSED_MAGIC2 = 0x42af9021; // random number
const DAVA::uint32 COMPRESSED_HEADER_SZ = 3; // two magic uint32s plus a uint32 for size

#define BLENDS_SZ 4

enum GroundType
{
    GROUNDTYPE_NONE = 0,
    GROUNDTYPE_FIRM = 1,
    GROUNDTYPE_MEDIUM = 2,
    GROUNDTYPE_SOFT = 3,
    GROUNDTYPE_SLOPE = 4,
    GROUNDTYPE_DEATH_ZONE = 5
};

class Converter final
{
public:
    Converter(const DAVA::FilePath& templatesPath, const DAVA::FilePath& clientDataSource);

    void Do(const DAVA::FilePath& filePath, const DAVA::Vector<DAVA::String>& tags);

private:
    DAVA::FilePath templatesPath;
    DAVA::FilePath clientDataSource;

private:
    void DoConvert(DAVA::FilePath sceneFile, const DAVA::Vector<DAVA::String>& tags);
    void DoConvertHM(void);
    void DoConvertModels(const DAVA::Vector<DAVA::String>& tags);
    void AddEntityToChunk(char* textBuf, const DAVA::Matrix4& wt, const DAVA::uint32 nodeId);
    void AddModelToChunk(DAVA::Entity* node, Model* model);
    void AddWater(DAVA::Entity* node, const DAVA::String& guid);
    void AddTreeToChunk(DAVA::Entity* node, ServerSPT* tree);
    void AddPointToChunk(DAVA::Entity* node);
    void AddWayPointToChunk(DAVA::PathComponent::Waypoint* wp, DAVA::uint32 pointNumber, DAVA::Entity* parentEntity);
    void GenerateMaterialKindMap(void);

    void SetupVersionLKA();

    UniqueID GetEntityID(DAVA::Entity* node);
    UniqueID GetEntityID(DAVA::Entity* node, DAVA::uint32 index);

    DAVA::Scene* scene = nullptr;
    DAVA::Landscape* landscapeObject = nullptr;
    DAVA::Entity* land = nullptr;
    DAVA::Vector<DAVA::int32> image32;

    DAVA::Heightmap* heightMap = nullptr;
    DAVA::Image* tileMask = nullptr;

    void quantisedPNGCompress(void);
    BinaryPtr compressPNG(PNGImageData const& data);
    BinaryPtr compressImage(DAVA::uint8* image);

    DAVA::String outsideChunkIdentifier(DAVA::int32 gridX, DAVA::int32 gridZ);
    void chunkIndexesFromChunkID(int chunkID, int& chunkX, int& chunkZ);
    DAVA::uint32 chunkIDFromChunkIndexes(int chunkX, int chunkZ);
    void CreateBorderFile(void);

    void compressResult(DAVA::String archiveName, BinaryPtr hMap);
    void archiveFile(DAVA::String fileName, DAVA::FilePath filePath, zipFile& zf);
    void browseDir(DAVA::String dirName, zipFile& archive);

    void CreateSettingsFile(DAVA::int32 minx, DAVA::int32 miny, DAVA::int32 maxx, DAVA::int32 maxy);
    void CheckNodes(DAVA::Entity* curr, const DAVA::Vector<DAVA::String>& tags);
    void CheckWater(DAVA::Entity* curr, const DAVA::Vector<DAVA::String>& tags);
    void CheckUserNodes(DAVA::Entity* curr, const DAVA::Vector<DAVA::String>& tags);
    void CheckPath(DAVA::Entity* curr, const DAVA::Vector<DAVA::String>& tags);

    bool CheckTags(DAVA::Entity* curr, const DAVA::Vector<DAVA::String>& tags);

    bool GetCollisionType(DAVA::Entity* curr, eModelPreset& result);

    bool PrepareIndexTexture(void);
    bool SaveDominant(void);
    void CompressDominant(int level = DEFAULT_COMPRESSION);
    bool SaveBlend(DAVA::int32 index);

    std::map<DAVA::String, Model*> savedModels;
    std::map<DAVA::String, ServerSPT*> savedTrees;

    std::map<DAVA::String, int> chunkIndices;

    char modelContentFormatPattern[BUFFERS_SZ];
    char resultBuff[BUFFERS_SZ];
    char* chunkBuffer = nullptr;
    char spawnPointFormatPattern[BUFFERS_SZ];
    char controlPointFormatPattern[BUFFERS_SZ];
    char botSpawnFormatPattern[BUFFERS_SZ];
    char patrolFormatPattern[BUFFERS_SZ];
    char speedTreeFormatPattern[BUFFERS_SZ];
    char waterFormatPattern[BUFFERS_SZ];
    char waterVloFormatPattern[BUFFERS_SZ];
    char transformBuf[BUFFERS_SZ];

    DAVA::String strategyPointPattern;

    DAVA::KeyedArchive* sceneNodesLinks = nullptr;
    DAVA::Entity* root = nullptr;
    DAVA::EditorConfig* config = nullptr;
    DAVA::uint8* texData = nullptr;
    DAVA::int32 texDataWidth;
    DAVA::int32 texDataHeight;

    DAVA::uint32 dominantSize;
    DAVA::uint8* dominant = nullptr;
    DAVA::uint8* blends[BLENDS_SZ];

    BinaryBlock imageBlock;
    BinaryBlock dominantBlock;
    BinaryBlock blendBlock[BLENDS_SZ];

    DAVA::Vector<DAVA::String> textureNames;
    DAVA::uint8 defaultColorIndex;
    DAVA::FilePath inputFile;
    DAVA::FilePath projectPath;
    DAVA::String mapName;
    DAVA::uint32 nextid;

    std::unique_ptr<ConverterVisitorBase> arenaDefCompilator;
};