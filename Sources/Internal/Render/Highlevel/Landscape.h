#pragma once

#include "Base/BaseObject.h"
#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Base/Vector.h"
#include "Concurrency/Mutex.h"
#include "Functional/Signal.h"
#include "Reflection/Reflection.h"
#include "Render/RenderBase.h"
#include "Render/Highlevel/RenderObject.h"
#include "FileSystem/FilePath.h"
#include "MemoryManager/MemoryProfiler.h"
#include "LandscapeSubdivision.h"
#include "LandscapePageManager.h"
#include "DecorationData.h"

namespace DAVA
{
class LandscapeSystem;
class FoliageSystem;
class NMaterial;
class SerializationContext;
class Heightmap;
class LandscapePageManager;
class Window;

class Landscape : public RenderObject
{
    DAVA_ENABLE_CLASS_ALLOCATION_TRACKING(ALLOC_POOL_LANDSCAPE)

public:
    enum eLandscapeTexture : uint32
    {
        HEIGHTMAP_TEXTURE,
        TANGENT_TEXTURE,
        TILEMASK_TEXTURE
    };

    enum LandscapeQuality : uint32
    {
        Low,
        Full,
    };

    Landscape();
    virtual ~Landscape();

    static const int32 RENDER_PARCEL_SIZE_VERTICES = 129;
    static const int32 RENDER_PARCEL_SIZE_QUADS = (RENDER_PARCEL_SIZE_VERTICES - 1);
    static const int32 RENDER_PARCEL_AND = RENDER_PARCEL_SIZE_VERTICES - 2;
    static const int32 INITIAL_INDEX_BUFFER_CAPACITY = 20000;

    static const int32 TEXTURE_SIZE_FULL_TILED = 2048;
    static const int32 CUSTOM_COLOR_TEXTURE_SIZE = 2048;

    static const FastName FLAG_DECORATION_ORIENT_ON_LANDSCAPE;
    static const FastName FLAG_DECORATION_GPU_RANDOMIZATION;

    static const FastName PARAM_TEXTURE_TILING;
    static const FastName PARAM_DECORATION_LEVEL_COLOR;
    static const FastName PARAM_DECORATION_DECORATION_MASK;
    static const FastName PARAM_DECORATION_ORIENT_VALUE;

    static const FastName TEXTURE_COLOR;
    static const FastName TEXTURE_TILEMASK;
    static const FastName TEXTURE_ALBEDO_TILE0;
    static const FastName TEXTURE_ALBEDO_TILE1;
    static const FastName TEXTURE_ALBEDO_TILE2;
    static const FastName TEXTURE_ALBEDO_TILE3;
    static const FastName TEXTURE_NORMALMAP_TILE0;
    static const FastName TEXTURE_NORMALMAP_TILE1;
    static const FastName TEXTURE_NORMALMAP_TILE2;
    static const FastName TEXTURE_NORMALMAP_TILE3;
    static const FastName TEXTURE_TERRAIN;
    static const FastName TEXTURE_DECORATION;
    static const FastName TEXTURE_DECORATION_COLOR;

    const static FastName LANDSCAPE_QUALITY_NAME;
    const static FastName LANDSCAPE_QUALITY_VALUE_HIGH;

    enum RenderMode
    {
        RENDERMODE_NO_INSTANCING = 0,
        RENDERMODE_INSTANCING,
        RENDERMODE_INSTANCING_MORPHING,
    };

    struct LansdcapeRenderStats
    {
        uint32 landscapeTriangles = 0u;
        uint32 landscapePatches = 0u;
        uint32 landscapePages = 0u;

        uint32 decorationTriangles = 0u;
        uint32 decorationItems = 0u;
        uint32 decorationPatches = 0u;
        uint32 decorationPages = 0u;

        Vector<uint32> decorationLayerTriangles;
        Vector<uint32> decorationLayerItems;

        void Reset();
    };

    //LandscapeVertex used in GetLevel0Geometry() only
    struct LandscapeVertex
    {
        Vector3 position;
        Vector2 texCoord;
    };

    //TODO: move to Beast
    DAVA_DEPRECATED(bool GetLevel0Geometry(Vector<LandscapeVertex>& vertices, Vector<int32>& indices) const);

    /**
        \brief Builds landscape from heightmap image and bounding box of this landscape block
        \param[in] landscapeBox axial-aligned bounding box of the landscape block
     */
    virtual void BuildLandscapeFromHeightmapImage(const FilePath& heightmapPathname, const AABBox3& landscapeBox);

    /**
        \brief Function to receive pathname of heightmap object
        \returns pathname of heightmap
     */
    const FilePath& GetHeightmapPathname();
    void SetHeightmapPathname(const FilePath& newPath);

    float32 GetLandscapeSize() const;
    void SetLandscapeSize(float32 newSize);

    float32 GetLandscapeHeight() const;
    void SetLandscapeHeight(float32 newHeight);

    void GetDataNodes(Set<DataNode*>& dataNodes) override;

    void Save(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Load(KeyedArchive* archive, SerializationContext* serializationContext) override;

    bool PlacePoint(const Vector3& point, Vector3& result, Vector3* normal = 0) const;
    bool GetHeightAtPoint(const Vector3& point, float32& value) const;

    Heightmap* GetHeightmap();
    virtual void SetHeightmap(Heightmap* height);
    void UpdateHeightmap(const Vector<uint16>& data, const Rect2i& rect);

    uint32 GetPageMaterialCount(uint32 layerIndex) const;
    NMaterial* GetPageMaterials(uint32 layerIndex = 0, uint32 materialIndex = 0) const;

    void SetPagesUpdatePerFrame(uint32 count);

    NMaterial* GetLandscapeMaterial();
    void SetLandscapeMaterial(NMaterial* material);

    void PrepareMaterial(NMaterial* material);

    RenderObject* Clone(RenderObject* newObject) override;
    void RecalcBoundingBox() override;

    const LansdcapeRenderStats& GetRenderStats() const;

    void BindDynamicParameters(Camera* camera, RenderBatch* batch) override;
    void PrepareToRender(Camera* camera) override;

    void UpdatePart(const Rect2i& rect);
    void SetUpdatable(bool isUpdatable);
    bool IsUpdatable() const;

    void InvalidateAllPages();
    void InvalidatePages(const Rect& rect, uint32 level = 0, uint32 x = 0, uint32 y = 0); //[0, 1]

    void SetForceMinSubdiv(uint32 level);

    void SetUseInstancing(bool useInstancing);
    bool IsUseInstancing() const;

    LandscapeSubdivision* GetSubdivision();

    RenderMode GetRenderMode() const;
    void SetRenderMode(RenderMode mode);
    void UpdateMaterialFlags();
    void SelectHeightmapTextureFormat();

    void RecursiveRayTrace(uint32 level, uint32 x, uint32 y, const Ray3& rayInObjectSpace, float32& resultT);
    bool RayTrace(const Ray3& rayInObjectSpace, float32& resultT);

    uint32 GetLayersCount() const;
    void SetLayersCount(uint32 count);

    const Vector<LandscapeLayerRenderer*>* GetTerrainLayerRenderers() const;
    void SetTerrainLayerRenderers(Vector<LandscapeLayerRenderer*>* layerRenderers);

    //#decoration
    DecorationData* GetDecorationData();
    void ReloadDecorationData();

    void SetRenderSystem(RenderSystem* renderSystem) override;

    int32 GetTextureCount(eLandscapeTexture textureSemantic) const;
    Texture* GetTexture(eLandscapeTexture textureSemantic, int32 index) const;
    void SetTexture(eLandscapeTexture textureSemantic, int32 index, Texture* texture);

    void SetPageUpdateLocked(bool locked);
    bool GetPageUpdateLocked() const;

    void SetDrawLandscapeGeometryEnabled(bool enabled);

protected:
    void AddPatchToRender(const LandscapeSubdivision::SubdivisionPatch* subdivPatch);
    void RequestPages(const LandscapeSubdivision::SubdivisionPatch* subdivPatch);
    const LandscapeSubdivision::SubdivisionPatch* GetLastTerminatedPatch(uint32 level, uint32 x, uint32 y);
    uint32 GetSubdivPatchCount(const LandscapeSubdivision::SubdivisionPatch* subdivPatch);

    void AllocateGeometryData();
    void ReleaseGeometryData();

    void RestoreGeometry();

    void SetLandscapeSize(const Vector3& newSize);
    bool BuildHeightmap();
    void RebuildLandscape();

    int32 GetHeightmapSize() const;

    void SetMaxTexturingLevel(uint32 level);
    uint32 GetMaxTexturingLevel() const;

    void SetTessellationLevels(uint32 levels);
    uint32 GetTessellationLevels() const;

    void SetTessellationHeight(float32 height);
    float32 GetTessellationHeight() const;

    void UpdateMaxSubdivisionLevel();

    void SetMicroTessellation(bool enabled);
    bool GetMicroTessellation() const;

    void SetDrawWired(bool isWire);
    bool IsDrawWired() const;

    void SetDrawMorphing(bool drawMorph);
    bool IsDrawMorphing() const;

    void SetDrawTessellationHeight(bool drawTessellationHeight);
    bool IsDrawTessellationHeight() const;

    void SetDrawVTPage(bool draw);
    bool IsDrawVTPage() const;

    void SetDrawPatches(bool draw);
    bool IsDrawPatches() const;

    void SetDrawDecorationLevels(bool draw);
    bool IsDrawDecorationLevels() const;

    void SetUseMorphing(bool useMorph);
    bool IsUseMorphing() const;

    void SetMiddleLODLevel(uint32 level);
    uint32 GetMiddleLODLevel() const;

    void SetMacroLODLevel(uint32 level);
    uint32 GetMacroLODLevel() const;

    void SetPageMaterialRefl(NMaterial* material);
    Vector<NMaterial*>& GetPageMaterialRefl() const;

    void SetPageMiddleMaterialRefl(Vector<NMaterial*>* material);
    Vector<NMaterial*>& GetPageMiddleMaterialRefl() const;

    void SetPageMacroMaterialRefl(Vector<NMaterial*>* material);
    Vector<NMaterial*>& GetPageMacroMaterialRefl() const;

    Vector3 CalculateNormal(uint32 x, uint32 y, uint32 step = 1) const;

    struct RestoreBufferData
    {
        enum eBufferType
        {
            RESTORE_BUFFER_VERTEX,
            RESTORE_BUFFER_INDEX,
            RESTORE_TEXTURE
        };

        rhi::Handle buffer;
        uint8* data;
        uint32 dataSize;
        uint32 level;
        eBufferType bufferType;
    };

    Mutex restoreDataMutex;
    Vector<RestoreBufferData> bufferRestoreData;

    FilePath heightmapPath;
    Heightmap* heightmap = nullptr;
    LandscapeSubdivision* subdivision = nullptr;

    NMaterial* landscapeMaterial = nullptr;
    NMaterial* decorationMaterial = nullptr;

    Vector2 heightmapSizeProperty;
    uint32 heightmapMaxBaseLod = 0;
    uint32 maxTexturingLevel = 10;
    uint32 tessellationLevelCount = 3;
    float32 tessellationHeight = 0.4f;

    LandscapeQuality quality = LandscapeQuality::Full;
    LansdcapeRenderStats renderStats;

    RenderMode renderMode = RENDERMODE_NO_INSTANCING;
    bool updatable = false;
    bool microtessellation = false;
    bool debugDrawMetrics = false;
    bool debugDrawMorphing = false;
    bool debugDrawTessellationHeight = false;
    bool debugDrawVTPages = false;
    bool debugDrawPatches = false;
    bool debugDrawVTexture = false;
    bool debugDrawDecorationLevels = false;
    bool debugDisableDecoration = false;
    bool lockPagesUpdate = false;
    bool drawLandscapeGeometry = true;

    void DebugDraw2D(Window*);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    //Non-instancing render

    struct VertexNoInstancing
    {
        Vector3 position;
        Vector2 texCoord;
        Vector3 normal;
        Vector3 tangent;
    };

    void AllocateGeometryDataNoInstancing();

    void AllocateRenderBatch();
    int16 AllocateParcelVertexBuffer(uint32 x, uint32 y, uint32 size);

    void DrawLandscapeNoInstancing();
    void DrawPatchNoInstancing(const LandscapeSubdivision::SubdivisionPatch* patch);

    void FlushQueue();

    inline uint16 GetVertexIndex(uint16 x, uint16 y);

    void ResizeIndicesBufferIfNeeded(DAVA::uint32 newSize);

    Vector<rhi::HVertexBuffer> vertexBuffers;
    std::vector<uint16> indices;

    uint32 vLayoutUIDNoInstancing = rhi::VertexLayout::InvalidUID;

    uint32 queueIndexCount = 0;
    int16 queuedQuadBuffer = 0;
    int32 flushQueueCounter = 0;

    uint32 quadsInWidthPow2 = 0;
    uint64 invalidateCallback = 0;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    //Instancing render

    struct VertexInstancing
    {
        Vector2 position;
        Vector2 edgeShiftDirection;
        Vector4 edgeMask;

        float32 edgeVertexIndex;
        float32 fence;
        Vector2 avgShift;
    };

    struct InstanceData
    {
        Vector2 patchOffset;
        float32 patchScale;
        float32 pageBlend;
        Vector4 neighbourPatchLodOffset; // per edge: left, right, bottom, top

        Vector2 vtPage0Offset;
        Vector2 vtPage0Scale;
        Vector2 vtPage1Offset;
        Vector2 vtPage1Scale;

        Vector4 neighbourPageBlend;

        //Members for morphing render-mode
        Vector4 neighbourPatchMorph;
        float32 patchLod;
        float32 patchMorph;
    };

    static const int32 INSTANCE_DATA_SIZE_MORPHING = sizeof(InstanceData);
    static const int32 INSTANCE_DATA_SIZE_NO_MORPHING = INSTANCE_DATA_SIZE_MORPHING - sizeof(Vector4) - 2 * sizeof(float32);

    struct InstanceDataBuffer
    {
        rhi::HVertexBuffer buffer;
        rhi::HSyncObject syncObject;
        uint32 bufferSize;
    };

    void AllocateGeometryDataInstancing();

    void CreateTextures();
    void CreateTextureData();
    void UpdateTextureData(const Rect2i& rect);
    void UpdateTextures();

    void DrawLandscapeInstancing();
    void DrawPatchInstancing(const LandscapeSubdivision::SubdivisionPatch* patch);

    InstanceDataBuffer* GetInstanceBuffer(uint32 bufferSize);

    Texture* heightTexture = nullptr;
    Texture* normalTexture = nullptr;
    rhi::TextureFormat heightTextureFormat = rhi::TEXTURE_FORMAT_R8G8B8A8;
    bool manualHeightInterpolation = false;

    Vector<Image*> heightTextureData;
    Vector<Image*> normalTextureData;

    rhi::HVertexBuffer patchVertexBuffer;
    rhi::HIndexBuffer patchIndexBuffer;
    uint32 instanceDataMaxSize = 128 * sizeof(InstanceData); //128 instances - initial value. It's will automatic enhanced if needed.
    uint32 INSTANCE_DATA_SIZE = 0;
    uint8* instanceDataPtr = nullptr;

    Vector<InstanceDataBuffer*> freeInstanceDataBuffers;
    Vector<InstanceDataBuffer*> usedInstanceDataBuffers;

    const LandscapeSubdivision::SubdivisionPatch* currentSubdivisionRoot = nullptr;
    uint32 currentTerminatedPatches = 0;
    LandscapePageManager* pageManager = nullptr;
    LandscapePageManager* decorationPageManager = nullptr;
    VirtualTexture* terrainVTexture = nullptr;
    VirtualTexture* decorationVTexture = nullptr;
    VTDecalPageRenderer* vtDecalRenderer = nullptr;

    uint32 maxPagesUpdatePerFrame = 0;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    //#decoration

    struct DecorationVertex
    {
        Vector3 position;

        Vector2 texCoord;
        float32 tint;
        float32 radius;

        Vector2 pivot;
        Vector2 circle;

        Vector3 normal;
        Vector3 tangent;
        Vector3 binormal;
    };

    struct InstanceDataDecoration
    {
        Vector2 patchOffset;
        float32 patchScale;
        float32 decorScale;

        Vector2 decorPageOffset;
        Vector2 decorPageScale;

        Vector3 flipRotate; //[sin, cos, y-flip]
        Vector2 randomRotate; //[sin, cos]
    };

    struct DecorationInstanceBuffer //one buffer per level, shared between all layers
    {
        rhi::HVertexBuffer instanceBuffer;
        uint32 instanceCount = 0;
        Vector<InstanceDataDecoration> instanceData;
    };

    struct DecorationBatch
    {
        RenderBatch* renderBatch = nullptr;
        uint32 itemsCount = 0u;
    };

    struct DecorationCollisionGroupData
    {
        Vector<Vector2> itemCoords;
        Vector<uint32> variations;
        float32 maxDensity = 0.f;
        float32 circlesRadius = 0.f;
    };

    struct DecorationItem
    {
        DecorationItem(const Vector2& center, float32 radius)
            : randomCircleCenter(center)
            , randomCircleRadius(radius)
        {
        }

        Vector2 randomCircleCenter;
        float32 randomCircleRadius = 0.f;
    };

    using DecorationVariationItems = Vector<DecorationItem>;
    using DecorationLevelItems = Vector<DecorationVariationItems>;

    void RebuildDecoration();
    void ReleaseDecorationRenderData();

    void DrawDecorationPatch(const LandscapeSubdivision::SubdivisionPatch* patch);

    float32 GetRandomFloat(uint32 layerIndex);
    Vector<DecorationLevelItems> GenerateCollisionFreeItems(uint32 layerIndex);
    Vector<DecorationLevelItems> GenerateRandomItems(uint32 layerIndex);

    uint32 vLayoutDecor = 0;
    DecorationData* decoration = nullptr;
    Vector<DecorationInstanceBuffer> decorationInstanceBuffers;
    Vector<Vector<DecorationBatch>> decorationBatches; //[layer][level]

    Vector<std::pair<Vector<float32>, uint32>> randomBuffer; //[random-values, head]

    uint32 layersCount = 1;
    Vector<LandscapeLayerRenderer*> landscapeLayerRenderers;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////

    friend class LandscapeSystem;

    DAVA_VIRTUAL_REFLECTION(Landscape, RenderObject);
};

// Inline functions
inline uint16 Landscape::GetVertexIndex(uint16 x, uint16 y)
{
    return x + y * RENDER_PARCEL_SIZE_VERTICES;
}

inline LandscapeSubdivision* Landscape::GetSubdivision()
{
    return subdivision;
}

inline Landscape::RenderMode Landscape::GetRenderMode() const
{
    return renderMode;
}

inline const Landscape::LansdcapeRenderStats& Landscape::GetRenderStats() const
{
    return renderStats;
}

inline void Landscape::SetPageUpdateLocked(bool locked)
{
    lockPagesUpdate = locked;
}

inline bool Landscape::GetPageUpdateLocked() const
{
    return lockPagesUpdate;
}

inline void Landscape::SetDrawLandscapeGeometryEnabled(bool enabled)
{
    drawLandscapeGeometry = enabled;
}
}
