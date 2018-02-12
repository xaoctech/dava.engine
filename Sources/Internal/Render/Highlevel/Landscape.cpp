#include "Base/UnordererMap.h"
#include "Time/SystemTimer.h"
#include "FileSystem/FileSystem.h"
#include "Utils/StringFormat.h"
#include "Utils/Random.h"
#include "Scene3D/Scene.h"
#include "Scene3D/SceneFileV2.h"
#include "Scene3D/Systems/FoliageSystem.h"
#include "Render/VirtualTexture.h"
#include "Render/Highlevel/Landscape.h"
#include "Render/Highlevel/Heightmap.h"
#include "Render/Highlevel/RenderPassNames.h"
#include "Render/Highlevel/LandscapeSubdivision.h"
#include "Render/Highlevel/LandscapePageManager.h"
#include "Render/Highlevel/LandscapeLayerRenderer.h"
#include "Render/Highlevel/LandscapePageRenderer.h"
#include "Render/Highlevel/VTDecalManager.h"
#include "Render/Highlevel/VTDecalPageRenderer.h"
#include "Render/Image/Image.h"
#include "Render/Image/ImageSystem.h"
#include "Render/Material/NMaterial.h"
#include "Render/Material/NMaterialNames.h"
#include "Render/Material/NMaterialManager.h"
#include "Render/RenderHelper.h"
#include "Render/Texture.h"
#include "Render/Renderer.h"
#include "Render/Shader.h"
#include "Render/ShaderCache.h"
#include "Render/TextureDescriptor.h"
#include "Render/DynamicBufferAllocator.h"
#include "Render/2D/Systems/RenderSystem2D.h"
#include "Scene3D/SceneFile/SerializationContext.h"
#include "Scene3D/Systems/QualitySettingsSystem.h"
#include "Debug/ProfilerCPU.h"
#include "Debug/ProfilerGPU.h"
#include "Debug/ProfilerMarkerNames.h"
#include "Concurrency/LockGuard.h"

#include "Engine/Engine.h"
#include "Engine/EngineSettings.h"

#include "Reflection/Reflection.h"
#include "Reflection/ReflectedMeta.h"
#include "Reflection/ReflectionRegistrator.h"

#include "Concurrency/Mutex.h"
#include "Concurrency/LockGuard.h"
#include "Logger/Logger.h"

#if defined(__DAVAENGINE_ANDROID__)
#include "Platform/DeviceInfo.h"
#endif
#include "CirclesPackings.h"

#include <random>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(Landscape)
{
    ReflectionRegistrator<Landscape>::Begin()
    .Field("heightmapPath", &Landscape::GetHeightmapPathname, &Landscape::SetHeightmapPathname)[M::DisplayName("Height Map Path")]
    .Field("size", &Landscape::GetLandscapeSize, static_cast<void (Landscape::*)(float32)>(&Landscape::SetLandscapeSize))[M::DisplayName("Size")]
    .Field("height", &Landscape::GetLandscapeHeight, &Landscape::SetLandscapeHeight)[M::DisplayName("Height")]
    .Field("userMorphing", &Landscape::IsUseMorphing, &Landscape::SetUseMorphing)[M::DisplayName("Use morphing")]
    .Field("tessellation", &Landscape::GetMicroTessellation, &Landscape::SetMicroTessellation)[M::DisplayName("Tessellation")]
    .Field("isDrawWired", &Landscape::IsDrawWired, &Landscape::SetDrawWired)[M::DisplayName("Is draw wired")]
    .Field("debugDrawMorphing", &Landscape::IsDrawMorphing, &Landscape::SetDrawMorphing)[M::DisplayName("Debug draw morphing")]
    .Field("debugDrawMetrics", &Landscape::debugDrawMetrics)[M::DisplayName("Debug draw metrics")]
    .Field("debugDrawVTPages", &Landscape::IsDrawVTPage, &Landscape::SetDrawVTPage)[M::DisplayName("Debug draw VTPages")]
    .Field("debugDrawPatches", &Landscape::IsDrawPatches, &Landscape::SetDrawPatches)[M::DisplayName("Debug draw Patches")]
    .Field("debugDrawVTexture", &Landscape::debugDrawVTexture)[M::DisplayName("Debug draw VTexture")]
    .Field("debugDrawTessellation", &Landscape::IsDrawTessellationHeight, &Landscape::SetDrawTessellationHeight)[M::DisplayName("Debug draw Tessellation")]
    .Field("debugDrawDecorationLevels", &Landscape::IsDrawDecorationLevels, &Landscape::SetDrawDecorationLevels)[M::DisplayName("Debug draw Deco-Levels")]
    .Field("debugDisableDecoration", &Landscape::debugDisableDecoration)[M::DisplayName("Debug disable decoration")]
    .Field("subdivision", &Landscape::subdivision)[M::DisplayName("Subdivision")]
    .Field("maxTexturingLevel", &Landscape::GetMaxTexturingLevel, &Landscape::SetMaxTexturingLevel)[M::DisplayName("Max Texturing Level"), M::Range(0, Any(), 1)]
    .Field("tessellationLevelCount", &Landscape::GetTessellationLevels, &Landscape::SetTessellationLevels)[M::DisplayName("Tessellation Levels"), M::Range(0, Any(), 1)]
    .Field("tessellationHeight", &Landscape::GetTessellationHeight, &Landscape::SetTessellationHeight)[M::DisplayName("Tessellation Height"), M::Range(0.f, 1.f, 0.01f)]
    .Field("middleLodLevel", &Landscape::GetMiddleLODLevel, &Landscape::SetMiddleLODLevel)[M::DisplayName("Middle LOD Level"), M::Range(0, Any(), 1)]
    .Field("macroLodLevel", &Landscape::GetMacroLODLevel, &Landscape::SetMacroLODLevel)[M::DisplayName("Macro LOD Level"), M::Range(0, Any(), 1)]
    .Field("layersCount", &Landscape::GetLayersCount, &Landscape::SetLayersCount)[M::DisplayName("Layers Count"), M::Range(1, 4, 1)]
    .Field("layerRenderers", &Landscape::GetTerrainLayerRenderers, &Landscape::SetTerrainLayerRenderers)[M::DisplayName("Layer Renderers"), M::ReadOnly()]
    .Field("landscapeMaterial", &Landscape::GetLandscapeMaterial, &Landscape::SetLandscapeMaterial)[M::DisplayName("Landscape Material"), M::ReadOnly()]
    .End();
}

const FastName Landscape::FLAG_DECORATION_ORIENT_ON_LANDSCAPE("ORIENT_ON_LANDSCAPE");
const FastName Landscape::FLAG_DECORATION_GPU_RANDOMIZATION("DECORATION_GPU_RANDOMIZATION");

const FastName Landscape::PARAM_TEXTURE_TILING("textureTiling");
const FastName Landscape::PARAM_DECORATION_LEVEL_COLOR("levelColor");
const FastName Landscape::PARAM_DECORATION_DECORATION_MASK("decorationmask");
const FastName Landscape::PARAM_DECORATION_ORIENT_VALUE("orientvalue");

const FastName Landscape::TEXTURE_COLOR("colortexture");
const FastName Landscape::TEXTURE_TILEMASK("tilemask");
const FastName Landscape::TEXTURE_ALBEDO_TILE0("albedoTile0");
const FastName Landscape::TEXTURE_ALBEDO_TILE1("albedoTile1");
const FastName Landscape::TEXTURE_ALBEDO_TILE2("albedoTile2");
const FastName Landscape::TEXTURE_ALBEDO_TILE3("albedoTile3");
const FastName Landscape::TEXTURE_NORMALMAP_TILE0("normalmapTile0");
const FastName Landscape::TEXTURE_NORMALMAP_TILE1("normalmapTile1");
const FastName Landscape::TEXTURE_NORMALMAP_TILE2("normalmapTile2");
const FastName Landscape::TEXTURE_NORMALMAP_TILE3("normalmapTile3");
const FastName Landscape::TEXTURE_TERRAIN("terraintexture");
const FastName Landscape::TEXTURE_DECORATION("decorationtexture");
const FastName Landscape::TEXTURE_DECORATION_COLOR("decorationcolortexture");

const FastName Landscape::LANDSCAPE_QUALITY_NAME("Landscape");
const FastName Landscape::LANDSCAPE_QUALITY_VALUE_HIGH("HIGH");

#define REDUCE_LANDSCAPE_QUALITY 0 /* possible values : 0 1 2 */

const uint32 LANDSCAPE_BATCHES_POOL_SIZE = 32;
const uint32 LANDSCAPE_MATERIAL_SORTING_KEY = 10;

static const uint32 PATCH_SIZE_VERTICES = 9;
static const uint32 PATCH_SIZE_QUADS = (PATCH_SIZE_VERTICES - 1);

static const uint32 INSTANCE_DATA_BUFFERS_POOL_SIZE = 9;

static const uint32 TERRAIN_VIRTUAL_TEXTURE_MIP_COUNT = 3 - REDUCE_LANDSCAPE_QUALITY;

static const uint32 TERRAIN_VIRTUAL_TEXTURE_WIDTH = 4096u >> REDUCE_LANDSCAPE_QUALITY;
static const uint32 TERRAIN_VIRTUAL_TEXTURE_HEIGHT = 2048u >> REDUCE_LANDSCAPE_QUALITY;
static const uint32 TERRAIN_VIRTUAL_TEXTURE_PAGE_SIZE = 128u >> REDUCE_LANDSCAPE_QUALITY;

static const uint32 DECORATION_VIRTUAL_TEXTURE_WIDTH = 2048u >> REDUCE_LANDSCAPE_QUALITY;
static const uint32 DECORATION_VIRTUAL_TEXTURE_HEIGHT = 1024u >> REDUCE_LANDSCAPE_QUALITY;
static const uint32 DECORATION_VIRTUAL_TEXTURE_PAGE_SIZE = 64u >> REDUCE_LANDSCAPE_QUALITY;

static const uint32 VT_PAGE_UPDATES_PER_FRAME_DEFAULT_COUNT = 16u;
static const uint32 VT_PAGE_LOD_COUNT = 3; //macro-, middle- and micro-tiles

static const uint32 LANDSCAPE_TESSELLATION_MODE_FLAG = 2;

#define LANDSCAPE_PATCH_FENCES 1

namespace LandscapeDetails
{
const Array<FastName, 4> regularTilemaskMatNames
{ {
FastName("TileMask_01_lod0"),
FastName("TileMask_02_lod0"),
FastName("TileMask_03_lod0"),
FastName("TileMask_04_lod0")
} };

const Array<FastName, 4> middleTilemaskMatNames
{ {
FastName("TileMask_01_lod1"),
FastName("TileMask_02_lod1"),
FastName("TileMask_03_lod1"),
FastName("TileMask_04_lod1"),
} };

const Array<FastName, 4> macroTilemaskMatNames
{ {
FastName("TileMask_01_lod2"),
FastName("TileMask_02_lod2"),
FastName("TileMask_03_lod2"),
FastName("TileMask_04_lod2"),
} };

const Array<const Array<FastName, 4>, 3> allTileMaskMaterialNames =
{ {
regularTilemaskMatNames,
middleTilemaskMatNames,
macroTilemaskMatNames
} };
}

void Landscape::LansdcapeRenderStats::Reset()
{
    landscapeTriangles = 0u;
    landscapePatches = 0u;

    decorationTriangles = 0u;
    decorationItems = 0u;
    decorationPatches = 0u;

    std::fill(decorationLayerTriangles.begin(), decorationLayerTriangles.end(), 0u);
    std::fill(decorationLayerItems.begin(), decorationLayerItems.end(), 0u);
}

Landscape::Landscape()
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    landscapeLayerRenderers.reserve(4);

    type = TYPE_LANDSCAPE;

    subdivision = new LandscapeSubdivision();
    decoration = new DecorationData();

    VirtualTexture::Descriptor vtDesc;
    vtDesc.width = TERRAIN_VIRTUAL_TEXTURE_WIDTH;
    vtDesc.height = TERRAIN_VIRTUAL_TEXTURE_HEIGHT;
    vtDesc.pageSize = TERRAIN_VIRTUAL_TEXTURE_PAGE_SIZE;
    vtDesc.virtualTextureLayers = { { FORMAT_RGBA8888, FORMAT_RGBA8888 } }; /* [albedo + height], [normal + roughness + shadow] */
    vtDesc.intermediateBuffers = { { FORMAT_RGBA8888, FORMAT_RGBA8888 } };
    vtDesc.mipLevelCount = TERRAIN_VIRTUAL_TEXTURE_MIP_COUNT;

    pageManager = new LandscapePageManager(vtDesc);
    terrainVTexture = pageManager->GetVirtualTexture();
    for (uint32 l = 0; l < terrainVTexture->GetLayersCount(); ++l)
    {
        terrainVTexture->GetLayerTexture(l)->SetMinMagFilter(rhi::TEXFILTER_LINEAR, rhi::TEXFILTER_LINEAR, rhi::TEXMIPFILTER_LINEAR);
        terrainVTexture->GetLayerTexture(l)->samplerState.anisotropyLevel = 1;
    }

    vtDesc.width = DECORATION_VIRTUAL_TEXTURE_WIDTH;
    vtDesc.height = DECORATION_VIRTUAL_TEXTURE_HEIGHT;
    vtDesc.pageSize = DECORATION_VIRTUAL_TEXTURE_PAGE_SIZE;
    vtDesc.virtualTextureLayers = { FORMAT_RGBA8888 };
    vtDesc.intermediateBuffers = { { FORMAT_RGBA8888, FORMAT_RGBA8888 } };
    vtDesc.mipLevelCount = 1;

    vtDecalRenderer = new VTDecalPageRenderer(false);

    DVASSERT(layersCount <= 4);
    for (uint8 i = 0; i < layersCount; ++i)
        landscapeLayerRenderers.push_back(new LandscapeLayerRenderer(VT_PAGE_LOD_COUNT));

    decorationPageManager = new LandscapePageManager(vtDesc);

    for (LandscapeLayerRenderer* rend : landscapeLayerRenderers)
        decorationPageManager->AddPageRenderer(rend);
    decorationPageManager->AddPageRenderer(vtDecalRenderer);

    decorationVTexture = decorationPageManager->GetVirtualTexture();
    decorationVTexture->GetLayerTexture(0)->SetMinMagFilter(rhi::TEXFILTER_LINEAR, rhi::TEXFILTER_LINEAR, rhi::TEXMIPFILTER_NONE);

    for (LandscapeLayerRenderer* rend : landscapeLayerRenderers)
        pageManager->AddPageRenderer(rend);
    pageManager->AddPageRenderer(vtDecalRenderer);

    maxPagesUpdatePerFrame = VT_PAGE_UPDATES_PER_FRAME_DEFAULT_COUNT;

    //GFX_COMPLETE
    //gap hard-coded now for tessellation. Later should be calculated accordion height of tessellation and decoration
    subdivision->SetPatchBBoxGap(tessellationHeight, tessellationHeight);

    renderMode = RENDERMODE_NO_INSTANCING;
    if (rhi::DeviceCaps().isInstancingSupported && rhi::DeviceCaps().isVertexTextureUnitsSupported)
    {
        if (rhi::TextureFormatSupported(rhi::TEXTURE_FORMAT_R8G8B8A8, rhi::PROG_VERTEX))
        {
            renderMode = RENDERMODE_INSTANCING_MORPHING;
        }
        else if (rhi::TextureFormatSupported(rhi::TEXTURE_FORMAT_R4G4B4A4, rhi::PROG_VERTEX))
        {
            renderMode = RENDERMODE_INSTANCING;
        }
        else if (rhi::TextureFormatSupported(rhi::TEXTURE_FORMAT_R32F, rhi::PROG_VERTEX))
        {
            renderMode = RENDERMODE_INSTANCING;
            floatHeightTexture = true;
        }
    }

    EngineSettings* settings = GetEngineContext()->settings;
    EngineSettings::eSettingValue landscapeSetting = settings->GetSetting<EngineSettings::SETTING_LANDSCAPE_RENDERMODE>().Get<EngineSettings::eSettingValue>();
    if (landscapeSetting == EngineSettings::LANDSCAPE_NO_INSTANCING)
        renderMode = RENDERMODE_NO_INSTANCING;
    else if (landscapeSetting == EngineSettings::LANDSCAPE_INSTANCING && renderMode == RENDERMODE_INSTANCING_MORPHING)
        renderMode = RENDERMODE_INSTANCING;

    isRequireNormal = true; // (QualitySettingsSystem::Instance()->GetCurMaterialQuality(LANDSCAPE_QUALITY_NAME) == LANDSCAPE_QUALITY_VALUE_HIGH);

#if defined(__DAVAENGINE_ANDROID__)
    if (renderMode == RENDERMODE_INSTANCING_MORPHING)
    {
        String version = DeviceInfo::GetVersion();
        const char* dotChar = strchr(version.c_str(), '.');
        int32 majorVersion = (dotChar && dotChar != version.c_str()) ? atoi(dotChar - 1) : 0;

        bool maliT600series = strstr(rhi::DeviceCaps().deviceDescription, "Mali-T6") != nullptr;

        //Workaround for some mali drivers (Android 4.x + T6xx gpu): it does not support fetch from texture mips in vertex program
        renderMode = (majorVersion == 4 && maliT600series) ? RENDERMODE_INSTANCING : RENDERMODE_INSTANCING_MORPHING;
    }
    if (renderMode != RENDERMODE_NO_INSTANCING)
    {
        //Workaround for Lenovo P90: on this device vertex texture fetch is very slow
        //(relevant for Android 4.4.4, currently there is no update to Android 5.0)
        if (strstr(DeviceInfo::GetModel().c_str(), "Lenovo P90") != nullptr)
        {
            renderMode = RENDERMODE_NO_INSTANCING;
        }
    }
#endif

    AddFlag(RenderObject::CUSTOM_PREPARE_TO_RENDER);

    Renderer::GetSignals().needRestoreResources.Connect(this, &Landscape::RestoreGeometry);

    invalidateCallback = NMaterialManager::Instance().RegisterInvalidateCallback([this]() {
        InvalidateAllPages();
    });

    if (nullptr != GetPrimaryWindow())
    {
        GetPrimaryWindow()->draw.Connect(this, &Landscape::DebugDraw2D);
    }
}

Landscape::~Landscape()
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    NMaterialManager::Instance().UnregisterInvalidateCallback(invalidateCallback);
    ReleaseGeometryData();

    SafeRelease(heightmap);
    SafeDelete(decoration);

    SafeDelete(pageManager);
    SafeDelete(vtDecalRenderer);
    SafeDelete(subdivision);
    SafeRelease(landscapeMaterial);

    SafeRelease(decorationMaterial);
    SafeDelete(decorationPageManager);

    for (LandscapePageRenderer* lr : landscapeLayerRenderers)
        delete lr;
    landscapeLayerRenderers.clear();

    Renderer::GetSignals().needRestoreResources.Disconnect(this);

    if (nullptr != GetPrimaryWindow())
    {
        GetPrimaryWindow()->draw.Disconnect(this);
    }
}

void Landscape::SetRenderSystem(RenderSystem* renderSystem)
{
    RenderSystem* currRs = GetRenderSystem();
    if (currRs)
        currRs->GetVTDecalManager()->RemoveLandscape(this);
    if (renderSystem)
        renderSystem->GetVTDecalManager()->AddLandscape(this);

    vtDecalRenderer->SetVTDecalManager(renderSystem ? renderSystem->GetVTDecalManager() : nullptr);
    RenderObject::SetRenderSystem(renderSystem);
}

void Landscape::RestoreGeometry()
{
    LockGuard<Mutex> lock(restoreDataMutex);
    for (auto& restoreData : bufferRestoreData)
    {
        switch (restoreData.bufferType)
        {
        case RestoreBufferData::RESTORE_BUFFER_VERTEX:
            if (rhi::NeedRestoreVertexBuffer(static_cast<rhi::HVertexBuffer>(restoreData.buffer)))
                rhi::UpdateVertexBuffer(static_cast<rhi::HVertexBuffer>(restoreData.buffer), restoreData.data, 0, restoreData.dataSize);
            break;

        case RestoreBufferData::RESTORE_BUFFER_INDEX:
            if (rhi::NeedRestoreIndexBuffer(static_cast<rhi::HIndexBuffer>(restoreData.buffer)))
                rhi::UpdateIndexBuffer(static_cast<rhi::HIndexBuffer>(restoreData.buffer), restoreData.data, 0, restoreData.dataSize);
            break;

        case RestoreBufferData::RESTORE_TEXTURE:
            // if (rhi::NeedRestoreTexture(static_cast<rhi::HTexture>(restoreData.buffer)))
            // we are not checking condition above,
            // because texture is marked as restored immediately after updating zero level
            rhi::UpdateTexture(static_cast<rhi::HTexture>(restoreData.buffer), restoreData.data, restoreData.level);
            break;

        default:
            DVASSERT(0, "Invalid RestoreBufferData type");
        }
    }
}

void Landscape::ReleaseGeometryData()
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    ////General
    for (RenderBatchWithOptions& batch : renderBatchArray)
        batch.renderBatch->Release();

    renderBatchArray.clear();
    activeRenderBatchArray.clear();

    {
        LockGuard<Mutex> lock(restoreDataMutex);
        bufferRestoreData.clear();
    }

    ////Non-instanced data
    for (rhi::HVertexBuffer handle : vertexBuffers)
        rhi::DeleteVertexBuffer(handle);
    vertexBuffers.clear();

    indices.clear();

    subdivision->ReleaseInternalData();

    quadsInWidthPow2 = 0;

    ////Instanced data

    if (patchVertexBuffer)
    {
        rhi::DeleteVertexBuffer(patchVertexBuffer);
        patchVertexBuffer = rhi::HVertexBuffer();
    }

    if (patchIndexBuffer)
    {
        rhi::DeleteIndexBuffer(patchIndexBuffer);
        patchIndexBuffer = rhi::HIndexBuffer();
    }

    for (InstanceDataBuffer* buffer : freeInstanceDataBuffers)
    {
        rhi::DeleteVertexBuffer(buffer->buffer);
        SafeDelete(buffer);
    }
    freeInstanceDataBuffers.clear();

    for (InstanceDataBuffer* buffer : usedInstanceDataBuffers)
    {
        rhi::DeleteVertexBuffer(buffer->buffer);
        SafeDelete(buffer);
    }
    usedInstanceDataBuffers.clear();

    if (landscapeMaterial)
    {
        if (landscapeMaterial->HasLocalTexture(NMaterialTextureName::TEXTURE_HEIGHTMAP))
            landscapeMaterial->RemoveTexture(NMaterialTextureName::TEXTURE_HEIGHTMAP);

        if (landscapeMaterial->HasLocalTexture(NMaterialTextureName::TEXTURE_TANGENTMAP))
            landscapeMaterial->RemoveTexture(NMaterialTextureName::TEXTURE_TANGENTMAP);
    }

    for (Image* img : heightTextureData)
        img->Release();
    heightTextureData.clear();

    for (Image* img : normalTextureData)
        img->Release();
    normalTextureData.clear();

    SafeRelease(heightTexture);
    SafeRelease(normalTexture);
}

void Landscape::BuildLandscapeFromHeightmapImage(const FilePath& heightmapPathname, const AABBox3& _box)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    heightmapPath = heightmapPathname;
    BuildHeightmap();

    bbox = _box;

    RebuildLandscape();
}

void Landscape::RecalcBoundingBox()
{
    //do nothing, bbox setup in BuildLandscapeFromHeightmapImage()
}

bool Landscape::BuildHeightmap()
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    bool retValue = false;
    SafeRelease(heightmap);

    if (DAVA::TextureDescriptor::IsSourceTextureExtension(heightmapPath.GetExtension()))
    {
        Vector<Image*> imageSet;
        ImageSystem::Load(heightmapPath, imageSet);
        if (0 != imageSet.size())
        {
            if ((imageSet[0]->GetPixelFormat() != FORMAT_A8) && (imageSet[0]->GetPixelFormat() != FORMAT_A16))
            {
                Logger::Error("Image for landscape should be gray scale");
            }
            else
            {
                DVASSERT(imageSet[0]->GetWidth() == imageSet[0]->GetHeight());
                heightmap = new Heightmap();
                heightmap->BuildFromImage(imageSet[0]);
                retValue = true;
            }

            for_each(imageSet.begin(), imageSet.end(), SafeRelease<Image>);
        }
    }
    else if (heightmapPath.IsEqualToExtension(Heightmap::FileExtension()))
    {
        heightmap = new Heightmap();
        retValue = heightmap->Load(heightmapPath);
    }

    return retValue;
}

int32 Landscape::GetHeightmapSize() const
{
    if (heightmap != nullptr)
    {
        return heightmap->Size();
    }
    return 0;
}

void Landscape::AllocateGeometryData()
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    uint32 heightmapSize = GetHeightmapSize();
    if (heightmapSize == 0)
    {
        return;
    }

    uint32 minSubdivLevelSize = (renderMode == RENDERMODE_NO_INSTANCING) ? heightmapSize / RENDER_PARCEL_SIZE_QUADS : 0;
    uint32 minSubdivLevel = uint32(HighestBitIndex(minSubdivLevelSize));

    heightmapSizef = float32(heightmapSize);
    heightmapSizePow2f = float32(uint32(HighestBitIndex(heightmapSize)));
    heightmapMaxBaseLod = FastLog2(heightmap->Size() / PATCH_SIZE_QUADS) + 1;

    subdivision->BuildSubdivision(heightmap, bbox, PATCH_SIZE_QUADS);
    subdivision->SetMinSubdivisionLevel(minSubdivLevel);
    UpdateMaxSubdivisionLevel();

    pageManager->Invalidate();
    decorationPageManager->Invalidate();

    (renderMode == RENDERMODE_NO_INSTANCING) ? AllocateGeometryDataNoInstancing() : AllocateGeometryDataInstancing();
}

void Landscape::RebuildLandscape()
{
    using namespace LandscapeDetails;
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    DVASSERT(landscapeLayerRenderers[0]->GetLODCount() <= VT_PAGE_LOD_COUNT);
    for (uint32 l = 0; l < landscapeLayerRenderers.size(); ++l)
    {
        for (uint32 i = 0; i < landscapeLayerRenderers[l]->GetLODCount(); ++i)
        {
            if (landscapeLayerRenderers[l]->GetTerrainLODMaterial(i) == nullptr)
            {
                ScopedPtr<NMaterial> tilemaskMaterial(new NMaterial());

                tilemaskMaterial->SetMaterialName(allTileMaskMaterialNames[i][l]);
                tilemaskMaterial->SetFXName(NMaterialName::TILE_MASK);
                tilemaskMaterial->AddFlag(NMaterialFlagName::FLAG_USE_PREVIOUS_LANDSCAPE_LAYER, l != 0);
                tilemaskMaterial->AddFlag(NMaterialFlagName::FLAG_LANDSCAPE_VT_PAGE, debugDrawVTPages ? 1 : 0);
                landscapeLayerRenderers[l]->SetTerrainLODMaterial(i, tilemaskMaterial.get());
            }

            ScopedPtr<NMaterial> decorationTileMaterial(new NMaterial());
            decorationTileMaterial->AddFlag(NMaterialFlagName::FLAG_DECORATION, 1);
            decorationTileMaterial->AddFlag(NMaterialFlagName::FLAG_USE_PREVIOUS_LANDSCAPE_LAYER, l != 0);
            decorationTileMaterial->SetParent(landscapeLayerRenderers[l]->GetTerrainLODMaterial(i));
            landscapeLayerRenderers[l]->SetDecorationLODMaterial(i, decorationTileMaterial.get());
        }
    }

    if (landscapeMaterial == nullptr)
    {
        landscapeMaterial = new NMaterial();
        landscapeMaterial->SetMaterialName(FastName("Landscape_Material"));
        landscapeMaterial->SetFXName(NMaterialName::LANDSCAPE);
        landscapeMaterial->AddTexture(NMaterialTextureName::TEXTURE_ALBEDO, terrainVTexture->GetLayerTexture(0));
        landscapeMaterial->AddTexture(NMaterialTextureName::TEXTURE_NORMAL, terrainVTexture->GetLayerTexture(1));
        landscapeMaterial->AddTexture(TEXTURE_TERRAIN, terrainVTexture->GetLayerTexture(0));

        PrepareMaterial(landscapeMaterial);
    }

    if (decoration)
    {
        decorationMaterial = new NMaterial();
        decorationMaterial->SetMaterialName(FastName("Decoration_Material"));
        decorationMaterial->AddFlag(NMaterialFlagName::FLAG_DECORATION_DRAW_LEVELS, 0);
        decorationMaterial->AddTexture(TEXTURE_DECORATION, decorationVTexture->GetLayerTexture(0));
        decorationMaterial->SetParent(landscapeMaterial);
        decorationMaterial->SetRuntime(true);

        RebuildDecoration();
    }

    ReleaseGeometryData();
    AllocateGeometryData();
}

void Landscape::PrepareMaterial(NMaterial* material)
{
    material->AddFlag(NMaterialFlagName::FLAG_LANDSCAPE_USE_INSTANCING, (renderMode == RENDERMODE_NO_INSTANCING) ? 0 : 1);
    material->AddFlag(NMaterialFlagName::FLAG_LANDSCAPE_LOD_MORPHING, (renderMode == RENDERMODE_INSTANCING_MORPHING) ? 1 : 0);
    material->AddFlag(NMaterialFlagName::FLAG_LANDSCAPE_MORPHING_COLOR, debugDrawMorphing ? 1 : 0);
    material->AddFlag(NMaterialFlagName::FLAG_LANDSCAPE_TESSELLATION_COLOR, debugDrawTessellationHeight ? 1 : 0);
    material->AddFlag(NMaterialFlagName::FLAG_LANDSCAPE_PATCHES, debugDrawPatches ? 1 : 0);
    material->AddFlag(NMaterialFlagName::FLAG_HEIGHTMAP_FLOAT_TEXTURE, floatHeightTexture ? 1 : 0);
    material->AddFlag(NMaterialFlagName::FLAG_LANDSCAPE_MICRO_TESSELLATION, microtessellation ? LANDSCAPE_TESSELLATION_MODE_FLAG : 0);
}

void Landscape::CreateTextures()
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    DVASSERT(renderMode != RENDERMODE_NO_INSTANCING);
    DVASSERT(heightTexture == nullptr);
    DVASSERT(normalTexture == nullptr);

    CreateTextureData();
    UpdateTextureData(Rect2i(0, 0, heightmap->Size(), heightmap->Size()));

    heightTexture = Texture::CreateFromData(heightTextureData);
    heightTexture->texDescriptor->pathname = "memoryfile_landscape_height";
    heightTexture->SetWrapMode(rhi::TEXADDR_CLAMP, rhi::TEXADDR_CLAMP);

    normalTexture = Texture::CreateFromData(normalTextureData);
    normalTexture->texDescriptor->pathname = "memoryfile_landscape_normal";
    normalTexture->SetWrapMode(rhi::TEXADDR_CLAMP, rhi::TEXADDR_CLAMP);

    if (renderMode == RENDERMODE_INSTANCING_MORPHING)
    {
        heightTexture->SetMinMagFilter(rhi::TEXFILTER_LINEAR, rhi::TEXFILTER_LINEAR, rhi::TEXMIPFILTER_NEAREST);
        normalTexture->SetMinMagFilter(rhi::TEXFILTER_LINEAR, rhi::TEXFILTER_LINEAR, rhi::TEXMIPFILTER_NEAREST);
    }
    else
    {
        heightTexture->SetMinMagFilter(rhi::TEXFILTER_LINEAR, rhi::TEXFILTER_LINEAR, rhi::TEXMIPFILTER_NONE);
        normalTexture->SetMinMagFilter(rhi::TEXFILTER_LINEAR, rhi::TEXFILTER_LINEAR, rhi::TEXMIPFILTER_NONE);
    }

    LockGuard<Mutex> lock(restoreDataMutex);

    uint32 level = 0;
    for (Image* img : heightTextureData)
    {
        bufferRestoreData.emplace_back();
        RestoreBufferData& restore = bufferRestoreData.back();
        restore.bufferType = RestoreBufferData::RESTORE_TEXTURE;
        restore.buffer = heightTexture->handle;
        restore.dataSize = img->dataSize;
        restore.data = img->data;
        restore.level = level++;
    }

    level = 0;
    for (Image* img : normalTextureData)
    {
        bufferRestoreData.emplace_back();
        RestoreBufferData& restore = bufferRestoreData.back();
        restore.bufferType = RestoreBufferData::RESTORE_TEXTURE;
        restore.buffer = normalTexture->handle;
        restore.dataSize = img->dataSize;
        restore.data = img->data;
        restore.level = level++;
    }
}

void Landscape::CreateTextureData()
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    const uint32 hmSize = GetHeightmapSize();
    DVASSERT(IsPowerOf2(hmSize));
    DVASSERT(renderMode != RENDERMODE_NO_INSTANCING);

    heightTextureData.clear();
    normalTextureData.clear();

    if (renderMode == RENDERMODE_INSTANCING_MORPHING)
    {
        DVASSERT(rhi::TextureFormatSupported(rhi::TEXTURE_FORMAT_R8G8B8A8, rhi::PROG_VERTEX));

        heightTextureData.reserve(HighestBitIndex(hmSize));
        normalTextureData.reserve(HighestBitIndex(hmSize));

        uint32 mipSize = hmSize;
        uint32 mipLevel = 0;
        while (mipSize)
        {
            heightTextureData.push_back(Image::Create(mipSize, mipSize, FORMAT_RGBA8888));
            normalTextureData.push_back(Image::Create(mipSize, mipSize, FORMAT_RGBA8888));

            heightTextureData.back()->mipmapLevel = mipLevel;
            normalTextureData.back()->mipmapLevel = mipLevel;

            mipSize >>= 1;
            ++mipLevel;
        }
    }
    else
    {
        Image* heightImage = nullptr;
        if (floatHeightTexture)
        {
            DVASSERT(rhi::TextureFormatSupported(rhi::TEXTURE_FORMAT_R32F, rhi::PROG_VERTEX));
            heightImage = Image::Create(hmSize, hmSize, FORMAT_R32F);
        }
        else
        {
            DVASSERT(rhi::TextureFormatSupported(rhi::TEXTURE_FORMAT_R4G4B4A4, rhi::PROG_VERTEX));
            heightImage = Image::Create(hmSize, hmSize, FORMAT_RGBA4444);
        }
        heightTextureData.push_back(heightImage);

        if (isRequireNormal)
        {
            DVASSERT(rhi::TextureFormatSupported(rhi::TEXTURE_FORMAT_R4G4B4A4, rhi::PROG_VERTEX));
            normalTextureData.push_back(Image::Create(hmSize, hmSize, FORMAT_RGBA4444));
        }
    }
}

void Landscape::UpdateTextureData(const Rect2i& rect)
{
    if (renderMode == RENDERMODE_INSTANCING_MORPHING)
    {
        DVASSERT(rhi::TextureFormatSupported(rhi::TEXTURE_FORMAT_R8G8B8A8, rhi::PROG_VERTEX));

        uint32 mipCount = uint32(heightTextureData.size());
        for (uint32 mip = 0; mip < mipCount; ++mip)
        {
            Image* hMipImage = heightTextureData[mip];
            Image* nMipImage = isRequireNormal ? normalTextureData[mip] : nullptr;

            int32 step = 1 << mip;
            int32 mipSize = heightmap->Size() >> mip;

            Rect2i mipRect;
            mipRect.x = (rect.x >> mip) - step;
            mipRect.y = (rect.y >> mip) - step;
            mipRect.dx = (rect.dx >> mip) + step * 2;
            mipRect.dy = (rect.dy >> mip) + step * 2;

            mipRect.x = Max(0, mipRect.x);
            mipRect.y = Max(0, mipRect.y);
            mipRect.dx = Min(mipSize - mipRect.x, mipRect.dx);
            mipRect.dy = Min(mipSize - mipRect.y, mipRect.dy);

            for (int32 y = mipRect.y; y < (mipRect.y + mipRect.dy); ++y)
            {
                uint16 yy = y * step;
                uint16 y1 = yy;
                uint16 y2 = yy;
                if ((y & 0x1) && y != (mipSize - 1))
                {
                    y1 -= step;
                    y2 += step;
                }

                for (int32 x = mipRect.x; x < (mipRect.x + mipRect.dx); ++x)
                {
                    uint16 xx = x * step;
                    uint16 x1 = xx;
                    uint16 x2 = xx;
                    if ((x & 0x1) && x != (mipSize - 1))
                    {
                        x1 += step;
                        x2 -= step;
                    }

                    //Height
                    uint16 hAcc = heightmap->GetHeight(xx, yy);

                    uint16 h1 = heightmap->GetHeightClamp(x1, y1);
                    uint16 h2 = heightmap->GetHeightClamp(x2, y2);
                    uint16 hAvg = (h1 + h2) / 2;

                    int32 pixelIndex = (hMipImage->GetWidth() * y + x);
                    uint32* heightPixel = reinterpret_cast<uint32*>(hMipImage->data) + pixelIndex;
                    *heightPixel = (hAvg << 16) | hAcc;

                    //Normal
                    if (nMipImage)
                    {
                        Vector3 normal = CalculateNormal(xx, yy);
                        Vector3 normal1 = CalculateNormal(x1, y1);
                        Vector3 normal2 = CalculateNormal(x2, y2);

                        normal = normal * 0.5f + 0.5f;
                        uint8 nxAcc = uint8(normal.x * 255.f);
                        uint8 nyAcc = uint8(normal.y * 255.f);

                        normal = Normalize(normal1 + normal2) * 0.5f + 0.5f;
                        uint8 nxAvg = uint8(normal.x * 255.f);
                        uint8 nyAvg = uint8(normal.y * 255.f);

                        uint32* normalPixel = reinterpret_cast<uint32*>(nMipImage->data) + pixelIndex;
                        *normalPixel = (nyAvg << 24) | (nxAvg << 16) | (nyAcc << 8) | (nxAcc);
                    }
                }
            }
        }
    }
    else
    {
        DVASSERT(heightTextureData.size() == 1);
        DVASSERT(!isRequireNormal || normalTextureData.size() == 1);

        Image* heightImage = heightTextureData[0];
        Image* normalImage = isRequireNormal ? normalTextureData[0] : nullptr;
        DVASSERT(rhi::TextureFormatSupported(rhi::TEXTURE_FORMAT_R32F, rhi::PROG_VERTEX));

        for (int32 y = rect.y; y < (rect.y + rect.dy); ++y)
        {
            for (int32 x = rect.x; x < (rect.x + rect.dx); ++x)
            {
                int32 pixelIndex = (heightImage->GetWidth() * y + x);
                if (floatHeightTexture)
                {
                    float32* heightPixel = reinterpret_cast<float32*>(heightImage->data) + pixelIndex;
                    *heightPixel = float32(heightmap->GetHeight(x, y)) / Heightmap::MAX_VALUE;
                }
                else
                {
                    uint16* heightPixel = reinterpret_cast<uint16*>(heightImage->data) + pixelIndex;
                    *heightPixel = heightmap->GetHeight(x, y);
                }

                if (normalImage)
                {
                    uint16* normalPixel = reinterpret_cast<uint16*>(normalImage->data) + pixelIndex;

                    Vector3 normal = CalculateNormal(x, y) * 0.5f + 0.5f;
                    *normalPixel = (uint8(normal.y * 255.f) << 8) | uint8(normal.x * 255.f);
                }
            }
        }
    }
}

void Landscape::UpdateTextures()
{
    for (Image* img : heightTextureData)
        heightTexture->TexImage(img->mipmapLevel, img->width, img->height, img->data, img->dataSize, img->cubeFaceID);

    for (Image* img : normalTextureData)
        normalTexture->TexImage(img->mipmapLevel, img->width, img->height, img->data, img->dataSize, img->cubeFaceID);
}

Vector3 Landscape::CalculateNormal(uint32 _x, uint32 _y, uint32 step) const
{
    DVASSERT(heightmap);
    int32 x = int32(_x);
    int32 y = int32(_y);

    Vector3 position = heightmap->GetPoint(x, y, bbox);
    Vector3 right = heightmap->GetPoint(x + step, y, bbox);
    Vector3 bottom = heightmap->GetPoint(x, y + step, bbox);
    Vector3 left = heightmap->GetPoint(x - step, y, bbox);
    Vector3 top = heightmap->GetPoint(x, y - step, bbox);

    Vector3 normal0 = CrossProduct(top - position, right - position);
    Vector3 normal1 = CrossProduct(right - position, bottom - position);
    Vector3 normal2 = CrossProduct(bottom - position, left - position);
    Vector3 normal3 = CrossProduct(left - position, top - position);

    Vector3 normalAverage = normal0 + normal1 + normal2 + normal3;

    return Normalize(normalAverage);

    /*
     VS: Algorithm
     // # P.xy store the position for which we want to calculate the normals
     // # height() here is a function that return the height at a point in the terrain
     
     // read neighbor heights using an arbitrary small offset
     vec3 off = vec3(1.0, 1.0, 0.0);
     float hL = height(P.xy - off.xz);
     float hR = height(P.xy + off.xz);
     float hD = height(P.xy - off.zy);
     float hU = height(P.xy + off.zy);
     
     // deduce terrain normal
     N.x = hL - hR;
     N.y = hD - hU;
     N.z = 2.0;
     N = normalize(N);
     */
}

bool Landscape::GetHeightAtPoint(const Vector3& point, float32& value) const
{
    if ((point.x > bbox.max.x) || (point.x < bbox.min.x) || (point.y > bbox.max.y) || (point.y < bbox.min.y))
    {
        return false;
    }

    int32 hmSize = GetHeightmapSize();
    if (hmSize == 0)
    {
        Logger::Error("[Landscape::GetHeightAtPoint] Trying to get height at point using empty heightmap data!");
        return false;
    }

    float32 fx = static_cast<float32>(hmSize) * (point.x - bbox.min.x) / (bbox.max.x - bbox.min.x);
    float32 fy = static_cast<float32>(hmSize) * (point.y - bbox.min.y) / (bbox.max.y - bbox.min.y);
    int32 x = static_cast<int32>(fx);
    int32 y = static_cast<int32>(fy);

    Vector3 h00 = heightmap->GetPoint(x, y, bbox);
    Vector3 h01 = heightmap->GetPoint(x + 1, y, bbox);
    Vector3 h10 = heightmap->GetPoint(x, y + 1, bbox);
    Vector3 h11 = heightmap->GetPoint(x + 1, y + 1, bbox);

    float32 dx = fx - static_cast<float32>(x);
    float32 dy = fy - static_cast<float32>(y);
    float32 h0 = h00.z * (1.0f - dx) + h01.z * dx;
    float32 h1 = h10.z * (1.0f - dx) + h11.z * dx;
    value = (h0 * (1.0f - dy) + h1 * dy);

    return true;
}

bool Landscape::PlacePoint(const Vector3& worldPoint, Vector3& result, Vector3* normal) const
{
    result = worldPoint;

    if (GetHeightAtPoint(worldPoint, result.z) == false)
    {
        return false;
    }

    if (normal != nullptr)
    {
        const float32 normalDelta = 0.01f;
        Vector3 dx = result + Vector3(normalDelta, 0.0f, 0.0f);
        Vector3 dy = result + Vector3(0.0f, normalDelta, 0.0f);
        GetHeightAtPoint(dx, dx.z);
        GetHeightAtPoint(dy, dy.z);
        *normal = (dx - result).CrossProduct(dy - result);
        normal->Normalize();
    }

    return true;
};

void Landscape::AddPatchToRender(const LandscapeSubdivision::SubdivisionPatch* subdivPatch)
{
    if (subdivPatch == nullptr)
        return;

    uint32 level = subdivPatch->level;
    uint32 x = subdivPatch->x;
    uint32 y = subdivPatch->y;

    if (!subdivPatch->isTerminated)
    {
        AddPatchToRender(subdivPatch->children[0]);
        AddPatchToRender(subdivPatch->children[1]);
        AddPatchToRender(subdivPatch->children[2]);
        AddPatchToRender(subdivPatch->children[3]);
    }
    else
    {
        if (renderMode == RENDERMODE_NO_INSTANCING)
            DrawPatchNoInstancing(subdivPatch);
        else
            DrawPatchInstancing(subdivPatch);
    }

    //////////////////////////////////////////////////////////////////////////
    //#decoration
    if ((decoration != nullptr) && (!debugDisableDecoration))
        DrawDecorationPatch(subdivPatch);
    //////////////////////////////////////////////////////////////////////////
}

void Landscape::RequestPages(const LandscapeSubdivision::SubdivisionPatch* subdivPatch)
{
    if (subdivPatch == nullptr)
        return;

    pageManager->RequestPage(subdivPatch->level, subdivPatch->x, subdivPatch->y, subdivPatch->radiusError);
    if (decoration != nullptr && subdivPatch->level <= decoration->GetBaseLevel())
        decorationPageManager->RequestPage(subdivPatch->level, subdivPatch->x, subdivPatch->y, subdivPatch->radiusError);

    if (subdivPatch->level < maxTexturingLevel)
    {
        RequestPages(subdivPatch->children[0]);
        RequestPages(subdivPatch->children[1]);
        RequestPages(subdivPatch->children[2]);
        RequestPages(subdivPatch->children[3]);
    }
}

const LandscapeSubdivision::SubdivisionPatch* Landscape::GetLastTerminatedPatch(uint32 level, uint32 x, uint32 y)
{
    uint32 levelSize = LandscapeSubdivision::GetLevelSize(level);
    if (x >= levelSize || y >= levelSize)
        return nullptr;

    DVASSERT(currentSubdivisionRoot != nullptr);
    const LandscapeSubdivision::SubdivisionPatch* scanSubdivPatch = currentSubdivisionRoot;

    uint32 mask = 1 << (level - 1);
    uint32 i = 0;
    for (i = 0; i < level; ++i)
    {
        uint32 xCheck = x & mask;
        uint32 yCheck = y & mask;

        uint32 xy = (x & mask) | ((y & mask) << 1);
        xy = xy >> (level - i - 1);

        if (!scanSubdivPatch->children[xy])
            break;

        scanSubdivPatch = scanSubdivPatch->children[xy];
        mask = mask >> 1;
    }

    return (scanSubdivPatch->isTerminated) ? scanSubdivPatch : nullptr;
}

uint32 Landscape::GetSubdivPatchCount(const LandscapeSubdivision::SubdivisionPatch* subdivPatch)
{
    if (!subdivPatch)
        return 0;

    uint32 count = 1;
    count += GetSubdivPatchCount(subdivPatch->children[0]);
    count += GetSubdivPatchCount(subdivPatch->children[1]);
    count += GetSubdivPatchCount(subdivPatch->children[2]);
    count += GetSubdivPatchCount(subdivPatch->children[3]);

    return count;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////Non-instancing render

void Landscape::AllocateGeometryDataNoInstancing()
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    rhi::VertexLayout vLayout;
    vLayout.AddElement(rhi::VS_POSITION, 0, rhi::VDT_FLOAT, 3);
    vLayout.AddElement(rhi::VS_TEXCOORD, 0, rhi::VDT_FLOAT, 2);
    if (isRequireNormal)
    {
        vLayout.AddElement(rhi::VS_NORMAL, 0, rhi::VDT_FLOAT, 3);
        vLayout.AddElement(rhi::VS_TANGENT, 0, rhi::VDT_FLOAT, 3);
    }
    vLayoutUIDNoInstancing = rhi::VertexLayout::UniqueId(vLayout);

    for (uint32 i = 0; i < LANDSCAPE_BATCHES_POOL_SIZE; i++)
    {
        AllocateRenderBatch();
    }

    indices.resize(INITIAL_INDEX_BUFFER_CAPACITY);

    uint32 quadsInWidth = heightmap->Size() / RENDER_PARCEL_SIZE_QUADS;
    // For cases where landscape is very small allocate 1 VBO.
    if (quadsInWidth == 0)
        quadsInWidth = 1;

    quadsInWidthPow2 = uint32(HighestBitIndex(quadsInWidth));

    for (uint32 y = 0; y < quadsInWidth; ++y)
    {
        for (uint32 x = 0; x < quadsInWidth; ++x)
        {
            uint16 check = AllocateParcelVertexBuffer(x * RENDER_PARCEL_SIZE_QUADS, y * RENDER_PARCEL_SIZE_QUADS, RENDER_PARCEL_SIZE_QUADS);
            DVASSERT(check == uint16(x + y * quadsInWidth));
        }
    }
}

void Landscape::AllocateRenderBatch()
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    ScopedPtr<RenderBatch> batch(new RenderBatch());
    AddRenderBatch(batch);

    batch->SetMaterial(landscapeMaterial);
    batch->SetSortingKey(LANDSCAPE_MATERIAL_SORTING_KEY);

    batch->vertexLayoutId = vLayoutUIDNoInstancing;
    batch->vertexCount = RENDER_PARCEL_SIZE_VERTICES * RENDER_PARCEL_SIZE_VERTICES;
}

int16 Landscape::AllocateParcelVertexBuffer(uint32 quadX, uint32 quadY, uint32 quadSize)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    uint32 verticesCount = (quadSize + 1) * (quadSize + 1);
    uint32 vertexSize = sizeof(VertexNoInstancing);
    if (!isRequireNormal)
    {
        vertexSize -= sizeof(Vector3); // (Vertex::normal);
        vertexSize -= sizeof(Vector3); // (Vertex::tangent);
    }

    uint8* landscapeVertices = new uint8[verticesCount * vertexSize];
    uint32 index = 0;
    for (uint32 y = quadY; y < quadY + quadSize + 1; ++y)
    {
        for (uint32 x = quadX; x < quadX + quadSize + 1; ++x)
        {
            VertexNoInstancing* vertex = reinterpret_cast<VertexNoInstancing*>(&landscapeVertices[index * vertexSize]);
            vertex->position = heightmap->GetPoint(x, y, bbox);

            Vector2 texCoord = Vector2(x / heightmapSizef, 1.0f - y / heightmapSizef);
            vertex->texCoord = texCoord;

            if (isRequireNormal)
            {
                vertex->normal = CalculateNormal(x, y);
                vertex->tangent = Normalize(CrossProduct(Vector3(0.f, 1.f, 0.f), vertex->normal));
            }

            index++;
        }
    }

    uint32 vBufferSize = static_cast<uint32>(verticesCount * vertexSize);

    rhi::VertexBuffer::Descriptor desc;
    desc.size = vBufferSize;
    desc.initialData = landscapeVertices;
    if (updatable)
        desc.usage = rhi::USAGE_DYNAMICDRAW;
    else
        desc.usage = rhi::USAGE_STATICDRAW;

    rhi::HVertexBuffer vertexBuffer = rhi::CreateVertexBuffer(desc);
    vertexBuffers.push_back(vertexBuffer);

#if defined(__DAVAENGINE_IPHONE__)
    SafeDeleteArray(landscapeVertices);
#else
    LockGuard<Mutex> lock(restoreDataMutex);
    bufferRestoreData.push_back({ vertexBuffer, landscapeVertices, vBufferSize, 0, RestoreBufferData::RESTORE_BUFFER_VERTEX });
#endif

    return int16(vertexBuffers.size() - 1);
}

void Landscape::DrawLandscapeNoInstancing()
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    flushQueueCounter = 0;
    activeRenderBatchArray.clear();
    queuedQuadBuffer = -1;

    DVASSERT(queueIndexCount == 0);

    AddPatchToRender(currentSubdivisionRoot);
    FlushQueue();
}

void Landscape::DrawPatchNoInstancing(const LandscapeSubdivision::SubdivisionPatch* patch)
{
    uint32 level = patch->level;
    uint32 xx = patch->x;
    uint32 yy = patch->y;

    const LandscapeSubdivision::SubdivisionPatch* xNeg = GetLastTerminatedPatch(level, xx - 1, yy);
    const LandscapeSubdivision::SubdivisionPatch* yNeg = GetLastTerminatedPatch(level, xx, yy - 1);
    const LandscapeSubdivision::SubdivisionPatch* xPos = GetLastTerminatedPatch(level, xx + 1, yy);
    const LandscapeSubdivision::SubdivisionPatch* yPos = GetLastTerminatedPatch(level, xx, yy + 1);

    uint32 xNegLevel = xNeg ? xNeg->level : level;
    uint32 yNegLevel = yNeg ? yNeg->level : level;
    uint32 xPosLevel = xPos ? xPos->level : level;
    uint32 yPosLevel = yPos ? yPos->level : level;

    uint32 levelSize = LandscapeSubdivision::GetLevelSize(level);

    int32 dividerPow2 = level - quadsInWidthPow2;
    DVASSERT(dividerPow2 >= 0);
    uint16 quadBuffer = ((yy >> dividerPow2) << quadsInWidthPow2) + (xx >> dividerPow2);

    if ((quadBuffer != queuedQuadBuffer) && (queuedQuadBuffer != -1))
    {
        FlushQueue();
    }

    queuedQuadBuffer = quadBuffer;

    // Draw Middle
    uint32 realVertexCountInPatch = heightmap->Size() >> level;
    uint32 step = realVertexCountInPatch / PATCH_SIZE_QUADS;
    uint32 heightMapStartX = xx * realVertexCountInPatch;
    uint32 heightMapStartY = yy * realVertexCountInPatch;

    ResizeIndicesBufferIfNeeded(queueIndexCount + PATCH_SIZE_QUADS * PATCH_SIZE_QUADS * 6);

    uint16* indicesPtr = indices.data() + queueIndexCount;
    // Draw middle block
    {
        for (uint16 y = (heightMapStartY & RENDER_PARCEL_AND); y < (heightMapStartY & RENDER_PARCEL_AND) + realVertexCountInPatch; y += step)
        {
            for (uint16 x = (heightMapStartX & RENDER_PARCEL_AND); x < (heightMapStartX & RENDER_PARCEL_AND) + realVertexCountInPatch; x += step)
            {
                uint16 x0 = x;
                uint16 y0 = y;
                uint16 x1 = x + step;
                uint16 y1 = y + step;

                uint16 x0aligned = x0;
                uint16 y0aligned = y0;
                uint16 x1aligned = x1;
                uint16 y1aligned = y1;

                uint16 x0aligned2 = x0;
                uint16 y0aligned2 = y0;
                uint16 x1aligned2 = x1;
                uint16 y1aligned2 = y1;

                if (x == (heightMapStartX & RENDER_PARCEL_AND))
                {
                    uint16 alignMod = levelSize >> xNegLevel;
                    if (alignMod > 1)
                    {
                        y0aligned = y0 / (alignMod * step) * (alignMod * step);
                        y1aligned = y1 / (alignMod * step) * (alignMod * step);
                    }
                }

                if (y == (heightMapStartY & RENDER_PARCEL_AND))
                {
                    uint16 alignMod = levelSize >> yNegLevel;
                    if (alignMod > 1)
                    {
                        x0aligned = x0 / (alignMod * step) * (alignMod * step);
                        x1aligned = x1 / (alignMod * step) * (alignMod * step);
                    }
                }

                if (x == ((heightMapStartX & RENDER_PARCEL_AND) + realVertexCountInPatch - step))
                {
                    uint16 alignMod = levelSize >> xPosLevel;
                    if (alignMod > 1)
                    {
                        y0aligned2 = y0 / (alignMod * step) * (alignMod * step);
                        y1aligned2 = y1 / (alignMod * step) * (alignMod * step);
                    }
                }

                if (y == ((heightMapStartY & RENDER_PARCEL_AND) + realVertexCountInPatch - step))
                {
                    uint16 alignMod = levelSize >> yPosLevel;
                    if (alignMod > 1)
                    {
                        x0aligned2 = x0 / (alignMod * step) * (alignMod * step);
                        x1aligned2 = x1 / (alignMod * step) * (alignMod * step);
                    }
                }

                *indicesPtr++ = GetVertexIndex(x0aligned, y0aligned);
                *indicesPtr++ = GetVertexIndex(x1aligned, y0aligned2);
                *indicesPtr++ = GetVertexIndex(x0aligned2, y1aligned);

                *indicesPtr++ = GetVertexIndex(x1aligned, y0aligned2);
                *indicesPtr++ = GetVertexIndex(x1aligned2, y1aligned2);
                *indicesPtr++ = GetVertexIndex(x0aligned2, y1aligned);

                queueIndexCount += 6;
            }
        }
    }
}

void Landscape::FlushQueue()
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    if (queueIndexCount == 0)
        return;

    DVASSERT(queuedQuadBuffer != -1);

    uint16* indicesPtr = indices.data();
    while (queueIndexCount != 0)
    {
        DVASSERT(flushQueueCounter <= static_cast<int32>(renderBatchArray.size()));
        if (static_cast<int32>(renderBatchArray.size()) == flushQueueCounter)
        {
            AllocateRenderBatch();
        }

        DynamicBufferAllocator::AllocResultIB indexBuffer = DynamicBufferAllocator::AllocateIndexBuffer(queueIndexCount);
        DVASSERT(queueIndexCount >= indexBuffer.allocatedindices);
        uint32 allocatedIndices = indexBuffer.allocatedindices - indexBuffer.allocatedindices % 3; //in buffer must be completed triangles

        Memcpy(indexBuffer.data, indicesPtr, allocatedIndices * sizeof(uint16));
        RenderBatch* batch = renderBatchArray[flushQueueCounter].renderBatch;
        batch->indexBuffer = indexBuffer.buffer;
        batch->indexCount = allocatedIndices;
        batch->startIndex = indexBuffer.baseIndex;
        batch->vertexBuffer = vertexBuffers[queuedQuadBuffer];
        batch->perfQueryMarker = ProfilerGPUMarkerName::LANDSCAPE;

        activeRenderBatchArray.emplace_back(batch);

        queueIndexCount -= allocatedIndices;
        indicesPtr += allocatedIndices;

        renderStats.landscapeTriangles += allocatedIndices / 3;
        ++flushQueueCounter;
    }

    DVASSERT(queueIndexCount == 0);
    queuedQuadBuffer = -1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////Instancing render

void Landscape::AllocateGeometryDataInstancing()
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    CreateTextures();

    landscapeMaterial->AddTexture(NMaterialTextureName::TEXTURE_HEIGHTMAP, heightTexture);
    if (normalTexture)
        landscapeMaterial->AddTexture(NMaterialTextureName::TEXTURE_TANGENTMAP, normalTexture);

/////////////////////////////////////////////////////////////////

#if LANDSCAPE_PATCH_FENCES
    const uint32 FENCE_VERICES_COUNT = PATCH_SIZE_VERTICES * 4;
    const uint32 FENCE_INDICES_COUNT = PATCH_SIZE_QUADS * 4 * 6;
    const uint32 BASE_PATCH_VERTICES_OFFSET = PATCH_SIZE_VERTICES * PATCH_SIZE_VERTICES;
    const uint32 VERTICES_COUNT = BASE_PATCH_VERTICES_OFFSET + FENCE_VERICES_COUNT;
    const uint32 INDICES_COUNT = PATCH_SIZE_QUADS * PATCH_SIZE_QUADS * 6 + FENCE_INDICES_COUNT;
#else
    const uint32 VERTICES_COUNT = PATCH_SIZE_VERTICES * PATCH_SIZE_VERTICES;
    const uint32 INDICES_COUNT = PATCH_SIZE_QUADS * PATCH_SIZE_QUADS * 6;
#endif

    VertexInstancing* patchVertices = new VertexInstancing[VERTICES_COUNT];
    uint16* patchIndices = new uint16[INDICES_COUNT];
    uint16* indicesPtr = patchIndices;

    float32 quadSize = 1.f / PATCH_SIZE_QUADS;

    for (uint32 y = 0; y < PATCH_SIZE_VERTICES; ++y)
    {
        for (uint32 x = 0; x < PATCH_SIZE_VERTICES; ++x)
        {
            VertexInstancing& vertex = patchVertices[y * PATCH_SIZE_VERTICES + x];
            vertex.position = Vector2(x * quadSize, y * quadSize);
            vertex.edgeMask = Vector4(0.f, 0.f, 0.f, 0.f);
            vertex.edgeShiftDirection = Vector2(0.f, 0.f);
            vertex.edgeVertexIndex = 0.f;
            vertex.fence = 0.f;

            vertex.avgShift.x = (x & 1) ? -quadSize : 0.f;
            vertex.avgShift.y = (y & 1) ? quadSize : 0.f;

            if (x < (PATCH_SIZE_VERTICES - 1) && y < (PATCH_SIZE_VERTICES - 1))
            {
                *indicesPtr++ = (y + 0) * PATCH_SIZE_VERTICES + (x + 0);
                *indicesPtr++ = (y + 0) * PATCH_SIZE_VERTICES + (x + 1);
                *indicesPtr++ = (y + 1) * PATCH_SIZE_VERTICES + (x + 0);

                *indicesPtr++ = (y + 1) * PATCH_SIZE_VERTICES + (x + 0);
                *indicesPtr++ = (y + 0) * PATCH_SIZE_VERTICES + (x + 1);
                *indicesPtr++ = (y + 1) * PATCH_SIZE_VERTICES + (x + 1);
            }
        }
    }

    for (uint32 i = 1; i < PATCH_SIZE_QUADS; ++i)
    {
        //x = 0; y = i; left side of patch without corners
        patchVertices[i * PATCH_SIZE_VERTICES].edgeMask = Vector4(1.f, 0.f, 0.f, 0.f);
        patchVertices[i * PATCH_SIZE_VERTICES].edgeShiftDirection = Vector2(0.f, -1.f) / float32(PATCH_SIZE_QUADS);
        patchVertices[i * PATCH_SIZE_VERTICES].edgeVertexIndex = float32(i);

        //x = i; y = 0; bottom side of patch without corners
        patchVertices[i].edgeMask = Vector4(0.f, 1.f, 0.f, 0.f);
        patchVertices[i].edgeShiftDirection = Vector2(-1.f, 0.f) / float32(PATCH_SIZE_QUADS);
        patchVertices[i].edgeVertexIndex = float32(i);

        //x = PATCH_QUAD_COUNT; y = i; right side of patch without corners
        patchVertices[i * PATCH_SIZE_VERTICES + PATCH_SIZE_QUADS].edgeMask = Vector4(0.f, 0.f, 1.f, 0.f);
        patchVertices[i * PATCH_SIZE_VERTICES + PATCH_SIZE_QUADS].edgeShiftDirection = Vector2(0.f, 1.f) / float32(PATCH_SIZE_QUADS);
        patchVertices[i * PATCH_SIZE_VERTICES + PATCH_SIZE_QUADS].edgeVertexIndex = float32(PATCH_SIZE_QUADS - i);

        //x = i; y = PATCH_QUAD_COUNT; top side of patch without corners
        patchVertices[PATCH_SIZE_QUADS * PATCH_SIZE_VERTICES + i].edgeMask = Vector4(0.f, 0.f, 0.f, 1.f);
        patchVertices[PATCH_SIZE_QUADS * PATCH_SIZE_VERTICES + i].edgeShiftDirection = Vector2(1.f, 0.f) / float32(PATCH_SIZE_QUADS);
        patchVertices[PATCH_SIZE_QUADS * PATCH_SIZE_VERTICES + i].edgeVertexIndex = float32(PATCH_SIZE_QUADS - i);
    }

#if LANDSCAPE_PATCH_FENCES
    for (uint32 i = 0; i < PATCH_SIZE_VERTICES; ++i)
    {
        //x = 0; y = i; left side of patch
        patchVertices[BASE_PATCH_VERTICES_OFFSET + i] = patchVertices[i * PATCH_SIZE_VERTICES];
        patchVertices[BASE_PATCH_VERTICES_OFFSET + i].fence = 1.f;

        //x = i; y = 0; bottom side of patch
        patchVertices[BASE_PATCH_VERTICES_OFFSET + i + PATCH_SIZE_VERTICES] = patchVertices[i];
        patchVertices[BASE_PATCH_VERTICES_OFFSET + i + PATCH_SIZE_VERTICES].fence = 1.f;

        //x = PATCH_QUAD_COUNT; y = i; right side of patch
        patchVertices[BASE_PATCH_VERTICES_OFFSET + i + PATCH_SIZE_VERTICES * 2] = patchVertices[i * PATCH_SIZE_VERTICES + PATCH_SIZE_QUADS];
        patchVertices[BASE_PATCH_VERTICES_OFFSET + i + PATCH_SIZE_VERTICES * 2].fence = 1.f;

        //x = i; y = PATCH_QUAD_COUNT; top side of patch
        patchVertices[BASE_PATCH_VERTICES_OFFSET + i + PATCH_SIZE_VERTICES * 3] = patchVertices[PATCH_SIZE_QUADS * PATCH_SIZE_VERTICES + i];
        patchVertices[BASE_PATCH_VERTICES_OFFSET + i + PATCH_SIZE_VERTICES * 3].fence = 1.f;

        if (i != (PATCH_SIZE_VERTICES - 1))
        {
            *indicesPtr++ = BASE_PATCH_VERTICES_OFFSET + i;
            *indicesPtr++ = i * PATCH_SIZE_VERTICES;
            *indicesPtr++ = BASE_PATCH_VERTICES_OFFSET + i + 1;

            *indicesPtr++ = BASE_PATCH_VERTICES_OFFSET + i + 1;
            *indicesPtr++ = i * PATCH_SIZE_VERTICES;
            *indicesPtr++ = (i + 1) * PATCH_SIZE_VERTICES;

            *indicesPtr++ = BASE_PATCH_VERTICES_OFFSET + i + PATCH_SIZE_VERTICES;
            *indicesPtr++ = BASE_PATCH_VERTICES_OFFSET + (i + 1) + PATCH_SIZE_VERTICES;
            *indicesPtr++ = i;

            *indicesPtr++ = i;
            *indicesPtr++ = BASE_PATCH_VERTICES_OFFSET + (i + 1) + PATCH_SIZE_VERTICES;
            *indicesPtr++ = i + 1;

            *indicesPtr++ = i * PATCH_SIZE_VERTICES + PATCH_SIZE_QUADS;
            *indicesPtr++ = BASE_PATCH_VERTICES_OFFSET + i + PATCH_SIZE_VERTICES * 2;
            *indicesPtr++ = (i + 1) * PATCH_SIZE_VERTICES + PATCH_SIZE_QUADS;

            *indicesPtr++ = (i + 1) * PATCH_SIZE_VERTICES + PATCH_SIZE_QUADS;
            *indicesPtr++ = BASE_PATCH_VERTICES_OFFSET + i + PATCH_SIZE_VERTICES * 2;
            *indicesPtr++ = BASE_PATCH_VERTICES_OFFSET + (i + 1) + PATCH_SIZE_VERTICES * 2;

            *indicesPtr++ = PATCH_SIZE_QUADS * PATCH_SIZE_VERTICES + i;
            *indicesPtr++ = PATCH_SIZE_QUADS * PATCH_SIZE_VERTICES + (i + 1);
            *indicesPtr++ = BASE_PATCH_VERTICES_OFFSET + i + PATCH_SIZE_VERTICES * 3;

            *indicesPtr++ = BASE_PATCH_VERTICES_OFFSET + i + PATCH_SIZE_VERTICES * 3;
            *indicesPtr++ = PATCH_SIZE_QUADS * PATCH_SIZE_VERTICES + (i + 1);
            *indicesPtr++ = BASE_PATCH_VERTICES_OFFSET + (i + 1) + PATCH_SIZE_VERTICES * 3;
        }
    }
#endif

    /////////////////////////////////////////////////////////////////

    INSTANCE_DATA_SIZE = (renderMode == RENDERMODE_INSTANCING) ? INSTANCE_DATA_SIZE_NO_MORPHING : INSTANCE_DATA_SIZE_MORPHING;

    for (uint32 i = 0; i < INSTANCE_DATA_BUFFERS_POOL_SIZE; ++i)
    {
        rhi::VertexBuffer::Descriptor instanceBufferDesc;
        instanceBufferDesc.size = INSTANCE_DATA_SIZE;
        instanceBufferDesc.usage = rhi::USAGE_DYNAMICDRAW;
        instanceBufferDesc.needRestore = false;

        InstanceDataBuffer* instanceDataBuffer = new InstanceDataBuffer();
        instanceDataBuffer->bufferSize = instanceBufferDesc.size;
        instanceDataBuffer->buffer = rhi::CreateVertexBuffer(instanceBufferDesc);

        freeInstanceDataBuffers.push_back(instanceDataBuffer);
    }

    rhi::VertexBuffer::Descriptor vdesc;
    vdesc.size = VERTICES_COUNT * sizeof(VertexInstancing);
    vdesc.initialData = patchVertices;
    vdesc.usage = rhi::USAGE_STATICDRAW;
    patchVertexBuffer = rhi::CreateVertexBuffer(vdesc);

    rhi::IndexBuffer::Descriptor idesc;
    idesc.size = INDICES_COUNT * sizeof(uint16);
    idesc.initialData = patchIndices;
    idesc.usage = rhi::USAGE_STATICDRAW;
    patchIndexBuffer = rhi::CreateIndexBuffer(idesc);

#if defined(__DAVAENGINE_IPHONE__)
    SafeDeleteArray(patchVertices);
    SafeDeleteArray(patchIndices);
#else
    LockGuard<Mutex> lock(restoreDataMutex);
    bufferRestoreData.push_back({ patchVertexBuffer, reinterpret_cast<uint8*>(patchVertices), vdesc.size, 0, RestoreBufferData::RESTORE_BUFFER_VERTEX });
    bufferRestoreData.push_back({ patchIndexBuffer, reinterpret_cast<uint8*>(patchIndices), idesc.size, 0, RestoreBufferData::RESTORE_BUFFER_INDEX });
#endif

    RenderBatch* batch = new RenderBatch();
    batch->SetMaterial(landscapeMaterial);
    batch->SetSortingKey(LANDSCAPE_MATERIAL_SORTING_KEY);
    batch->vertexBuffer = patchVertexBuffer;
    batch->indexBuffer = patchIndexBuffer;
    batch->primitiveType = rhi::PRIMITIVE_TRIANGLELIST;
    batch->indexCount = INDICES_COUNT;
    batch->vertexCount = VERTICES_COUNT;

    rhi::VertexLayout vLayout;
    vLayout.AddStream(rhi::VDF_PER_VERTEX);
    vLayout.AddElement(rhi::VS_POSITION, 0, rhi::VDT_FLOAT, 4); //position + edgeShiftDirection
    vLayout.AddElement(rhi::VS_NORMAL, 0, rhi::VDT_FLOAT, 4); //edge mask
    vLayout.AddElement(rhi::VS_TANGENT, 0, rhi::VDT_FLOAT, 4); //vertex index + fence + avgShift
    vLayout.AddStream(rhi::VDF_PER_INSTANCE);
    vLayout.AddElement(rhi::VS_TEXCOORD, 0, rhi::VDT_FLOAT, 4); //patch position + scale + texture-page blend
    vLayout.AddElement(rhi::VS_TEXCOORD, 1, rhi::VDT_FLOAT, 4); //neighbor patch lodOffset
    vLayout.AddElement(rhi::VS_TEXCOORD, 2, rhi::VDT_FLOAT, 4); //texture coords0 offset + scale
    vLayout.AddElement(rhi::VS_TEXCOORD, 3, rhi::VDT_FLOAT, 4); //texture coords1 offset + scale
    vLayout.AddElement(rhi::VS_TEXCOORD, 4, rhi::VDT_FLOAT, 4); //neighbor patch page blend
    if (renderMode == RENDERMODE_INSTANCING_MORPHING)
    {
        vLayout.AddElement(rhi::VS_TEXCOORD, 5, rhi::VDT_FLOAT, 4); //neighbor patch morph
        vLayout.AddElement(rhi::VS_TEXCOORD, 6, rhi::VDT_FLOAT, 2); //patch lod + morph
    }

    batch->vertexLayoutId = rhi::VertexLayout::UniqueId(vLayout);

    AddRenderBatch(batch);
    SafeRelease(batch);
}

Landscape::InstanceDataBuffer* Landscape::GetInstanceBuffer(uint32 bufferSize)
{
    InstanceDataBuffer* instanceDataBuffer = nullptr;
    if (freeInstanceDataBuffers.size())
    {
        instanceDataBuffer = freeInstanceDataBuffers.back();
        if (instanceDataBuffer->bufferSize < bufferSize)
        {
            rhi::DeleteVertexBuffer(instanceDataBuffer->buffer);
            SafeDelete(instanceDataBuffer);
        }
        freeInstanceDataBuffers.pop_back();
    }

    if (!instanceDataBuffer)
    {
        instanceDataMaxSize = Max(instanceDataMaxSize, bufferSize);

        rhi::VertexBuffer::Descriptor instanceBufferDesc;
        instanceBufferDesc.size = instanceDataMaxSize;
        instanceBufferDesc.usage = rhi::USAGE_DYNAMICDRAW;
        instanceBufferDesc.needRestore = false;

        instanceDataBuffer = new InstanceDataBuffer();
        instanceDataBuffer->bufferSize = instanceBufferDesc.size;
        instanceDataBuffer->buffer = rhi::CreateVertexBuffer(instanceBufferDesc);
    }
    usedInstanceDataBuffers.push_back(instanceDataBuffer);
    instanceDataBuffer->syncObject = rhi::GetCurrentFrameSyncObject();

    return instanceDataBuffer;
}

void Landscape::DrawLandscapeInstancing()
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    activeRenderBatchArray.clear();

    for (int32 i = static_cast<int32>(usedInstanceDataBuffers.size()) - 1; i >= 0; --i)
    {
        if (rhi::SyncObjectSignaled(usedInstanceDataBuffers[i]->syncObject))
        {
            freeInstanceDataBuffers.push_back(usedInstanceDataBuffers[i]);
            RemoveExchangingWithLast(usedInstanceDataBuffers, i);
        }
    }

    if (currentTerminatedPatches)
    {
        RequestPages(currentSubdivisionRoot);
        if (!lockPagesUpdate)
        {
            Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_TESSELLATION_HEIGHT, &tessellationHeight, pointer_size(&tessellationHeight));
            pageManager->ProcessRequests(subdivision, maxPagesUpdatePerFrame, LandscapePageRenderer::eLandscapeComponent::COMPONENT_TERRAIN);
            decorationPageManager->ProcessRequests(subdivision, maxPagesUpdatePerFrame, LandscapePageRenderer::eLandscapeComponent::COMPONENT_DECORATION);
        }
        else
        {
            pageManager->RejectRequests();
            decorationPageManager->RejectRequests();
        }

        InstanceDataBuffer* instanceDataBuffer = GetInstanceBuffer(currentTerminatedPatches * INSTANCE_DATA_SIZE);
        renderBatchArray[0].renderBatch->instanceCount = currentTerminatedPatches;
        renderBatchArray[0].renderBatch->instanceBuffer = instanceDataBuffer->buffer;
        activeRenderBatchArray.emplace_back(renderBatchArray[0].renderBatch);

        instanceDataPtr = static_cast<uint8*>(rhi::MapVertexBuffer(instanceDataBuffer->buffer, 0, currentTerminatedPatches * INSTANCE_DATA_SIZE));

        AddPatchToRender(currentSubdivisionRoot);

        rhi::UnmapVertexBuffer(instanceDataBuffer->buffer);
        instanceDataPtr = nullptr;

        //////////////////////////////////////////////////////////////////////////
        //#decoration

        if (decoration)
        {
            for (DecorationInstanceBuffer& buffer : decorationInstanceBuffers)
            {
                if (buffer.instanceCount)
                {
                    uint32 instDataSize = buffer.instanceCount * sizeof(InstanceDataDecoration);
                    InstanceDataBuffer* instBuffer = GetInstanceBuffer(instDataSize);

                    uint8* instDataPrt = static_cast<uint8*>(rhi::MapVertexBuffer(instBuffer->buffer, 0, instDataSize));
                    Memcpy(instDataPrt, buffer.instanceData.data(), instDataSize);
                    rhi::UnmapVertexBuffer(instBuffer->buffer);

                    buffer.instanceBuffer = instBuffer->buffer;
                }
                buffer.instanceData.clear();
            }

            for (size_t l = 0; l < decorationBatches.size(); ++l)
            {
                Vector<DecorationBatch>& batches = decorationBatches[l];

                if (batches.empty())
                    continue;

                for (uint32 j = 0; j < decoration->GetLevelCount(); ++j)
                {
                    RenderBatch* batch = batches[j].renderBatch;
                    if (batch)
                    {
                        batch->instanceCount = decorationInstanceBuffers[j].instanceCount;
                        batch->instanceBuffer = decorationInstanceBuffers[j].instanceBuffer;

                        renderStats.decorationTriangles += batch->indexCount * batch->instanceCount / 3;
                        renderStats.decorationItems += batches[j].itemsCount * batch->instanceCount;
                        renderStats.decorationPatches += batch->instanceCount;

                        renderStats.decorationLayerTriangles[l] += batch->indexCount * batch->instanceCount / 3;
                        renderStats.decorationLayerItems[l] += batches[j].itemsCount * batch->instanceCount;

                        batch->perfQueryMarker = ProfilerGPUMarkerName::DECORATION;

                        activeRenderBatchArray.push_back(batch);
                    }
                }
            }

            for (DecorationInstanceBuffer& buffer : decorationInstanceBuffers)
            {
                buffer.instanceCount = 0;
                buffer.instanceBuffer = rhi::HVertexBuffer();
            }
        }

        //////////////////////////////////////////////////////////////////////////

        renderStats.landscapeTriangles = activeRenderBatchArray[0]->indexCount * activeRenderBatchArray[0]->instanceCount / 3;
        renderStats.landscapePatches = activeRenderBatchArray[0]->instanceCount;

        activeRenderBatchArray[0]->perfQueryMarker = ProfilerGPUMarkerName::LANDSCAPE;
    }
}

inline float32 morphFunc(float32 x)
{
    float32 _x = 1.f - x;
    float32 _x4 = _x * _x; //(1 - x) ^ 2
    _x4 = _x4 * _x4; //(1 - x) ^ 4

    return _x4 * (4.f * _x - 5.f) + 1; //4*(1-x)^5 - 5*(1-x)^4 + 1
}

inline Vector4 morphFunc(const Vector4& v)
{
    return Vector4(morphFunc(v.x), morphFunc(v.y), morphFunc(v.z), morphFunc(v.w));
}

void Landscape::DrawPatchInstancing(const LandscapeSubdivision::SubdivisionPatch* patch)
{
    uint32 level = patch->level;
    uint32 x = patch->x;
    uint32 y = patch->y;

    const LandscapeSubdivision::SubdivisionPatch* xNeg = GetLastTerminatedPatch(level, x - 1, y);
    const LandscapeSubdivision::SubdivisionPatch* yNeg = GetLastTerminatedPatch(level, x, y - 1);
    const LandscapeSubdivision::SubdivisionPatch* xPos = GetLastTerminatedPatch(level, x + 1, y);
    const LandscapeSubdivision::SubdivisionPatch* yPos = GetLastTerminatedPatch(level, x, y + 1);

    uint32 xNegLevel = xNeg ? xNeg->level : level;
    uint32 yNegLevel = yNeg ? yNeg->level : level;
    uint32 xPosLevel = xPos ? xPos->level : level;
    uint32 yPosLevel = yPos ? yPos->level : level;

    InstanceData* instanceData = reinterpret_cast<InstanceData*>(instanceDataPtr);

    //patch params
    {
        float32 levelSizef = float32(LandscapeSubdivision::GetLevelSize(level));

        instanceData->patchOffset = Vector2(x / levelSizef, y / levelSizef);
        instanceData->patchScale = 1.f / levelSizef;
        instanceData->pageBlend = patch->morphCoeff;
        instanceData->neighbourPatchLodOffset = Vector4(float32(level - xNegLevel),
                                                        float32(level - yNegLevel),
                                                        float32(level - xPosLevel),
                                                        float32(level - yPosLevel)
                                                        );

        LandscapePageManager::PageMapping pageMapping = pageManager->GetSuitablePage(level, x, y);
        instanceData->vtPage0Offset = pageMapping.uvOffset0;
        instanceData->vtPage0Scale = pageMapping.uvScale0;
        instanceData->vtPage1Offset = pageMapping.uvOffset1;
        instanceData->vtPage1Scale = pageMapping.uvScale1;
    }

    //morphing params
    if (renderMode == RENDERMODE_INSTANCING_MORPHING || microtessellation)
    {
        float32 patchMorph = patch->morphCoeff;
        float32 xNegMorph = xNeg ? xNeg->morphCoeff : patchMorph;
        float32 yNegMorph = yNeg ? yNeg->morphCoeff : patchMorph;
        float32 xPosMorph = xPos ? xPos->morphCoeff : patchMorph;
        float32 yPosMorph = yPos ? yPos->morphCoeff : patchMorph;

        xNegMorph = (xNegLevel < level) ? xNegMorph : Min(xNegMorph, patchMorph);
        yNegMorph = (yNegLevel < level) ? yNegMorph : Min(yNegMorph, patchMorph);
        xPosMorph = (xPosLevel < level) ? xPosMorph : Min(xPosMorph, patchMorph);
        yPosMorph = (yPosLevel < level) ? yPosMorph : Min(yPosMorph, patchMorph);

        instanceData->neighbourPageBlend = Vector4(xNegMorph, yNegMorph, xPosMorph, yPosMorph);

        if (renderMode == RENDERMODE_INSTANCING_MORPHING)
        {
            patchMorph = (level >= heightmapMaxBaseLod) ? 1.f : patchMorph;
            xNegMorph = (xNegLevel >= heightmapMaxBaseLod) ? 1.f : xNegMorph;
            yNegMorph = (yNegLevel >= heightmapMaxBaseLod) ? 1.f : yNegMorph;
            xPosMorph = (xPosLevel >= heightmapMaxBaseLod) ? 1.f : xPosMorph;
            yPosMorph = (yPosLevel >= heightmapMaxBaseLod) ? 1.f : yPosMorph;

            instanceData->patchLod = float32(int32(heightmapMaxBaseLod) - int32(level) - 1);
            instanceData->patchMorph = morphFunc(patchMorph);
            instanceData->neighbourPatchMorph = morphFunc(Vector4(xNegMorph, yNegMorph, xPosMorph, yPosMorph));
        }
    }

    instanceDataPtr += INSTANCE_DATA_SIZE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////

void Landscape::BindDynamicParameters(Camera* camera, RenderBatch* batch)
{
    RenderObject::BindDynamicParameters(camera, batch);

    if (heightmap)
    {
        Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_LANDSCAPE_HEIGHTMAP_SIZE_POW2, &heightmapSizePow2f, pointer_size(&heightmapSizePow2f));
        Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_LANDSCAPE_HEIGHTMAP_SIZE, &heightmapSizef, pointer_size(&heightmapSizef));
        Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_TESSELLATION_HEIGHT, &tessellationHeight, pointer_size(&tessellationHeight));
    }
}

void Landscape::PrepareToRender(Camera* camera)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();
    DAVA_PROFILER_CPU_SCOPE(ProfilerCPUMarkerName::RENDER_PREPARE_LANDSCAPE);

    if (decoration && decoration->paramsChanged)
    {
        RebuildDecoration();
        decoration->paramsChanged = false;
    }

    RenderObject::PrepareToRender(camera);

    if (GetHeightmapSize() == 0)
        return;

    if (!Renderer::GetOptions()->IsOptionEnabled(RenderOptions::LANDSCAPE_DRAW))
        return;

    if (Renderer::GetOptions()->IsOptionEnabled(RenderOptions::UPDATE_LANDSCAPE_LODS))
        currentSubdivisionRoot = subdivision->PrepareSubdivision(camera, worldTransform, &currentTerminatedPatches);

    renderStats.Reset();

    switch (renderMode)
    {
    case RENDERMODE_INSTANCING:
    case RENDERMODE_INSTANCING_MORPHING:
        DrawLandscapeInstancing();
        break;
    case RENDERMODE_NO_INSTANCING:
        DrawLandscapeNoInstancing();
    default:
        break;
    }

    currentSubdivisionRoot = nullptr;
    currentTerminatedPatches = 0;
}

bool Landscape::GetLevel0Geometry(Vector<LandscapeVertex>& vertices, Vector<int32>& indices) const
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();
    uint32 hmSize = GetHeightmapSize();
    if (hmSize == 0)
    {
        return false;
    }

    uint32 gridWidth = hmSize + 1;
    uint32 gridHeight = hmSize + 1;
    vertices.resize(gridWidth * gridHeight);
    for (uint32 y = 0, index = 0; y < gridHeight; ++y)
    {
        float32 ny = static_cast<float32>(y) / static_cast<float32>(gridHeight - 1);
        for (uint32 x = 0; x < gridWidth; ++x)
        {
            float32 nx = static_cast<float32>(x) / static_cast<float32>(gridWidth - 1);
            vertices[index].position = heightmap->GetPoint(x, y, bbox);
            vertices[index].texCoord = Vector2(nx, ny);
            index++;
        }
    }

    indices.resize((gridWidth - 1) * (gridHeight - 1) * 6);
    for (uint32 y = 0, index = 0; y < gridHeight - 1; ++y)
    {
        for (uint32 x = 0; x < gridWidth - 1; ++x)
        {
            indices[index++] = x + y * gridWidth;
            indices[index++] = (x + 1) + y * gridWidth;
            indices[index++] = x + (y + 1) * gridWidth;
            indices[index++] = (x + 1) + y * gridWidth;
            indices[index++] = (x + 1) + (y + 1) * gridWidth;
            indices[index++] = x + (y + 1) * gridWidth;
        }
    }
    return true;
}

const FilePath& Landscape::GetHeightmapPathname()
{
    return heightmapPath;
}

void Landscape::SetHeightmapPathname(const FilePath& newHeightMapPath)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    if (newHeightMapPath == heightmapPath)
    {
        return;
    }
    BuildLandscapeFromHeightmapImage(newHeightMapPath, bbox);
}

float32 Landscape::GetLandscapeSize() const
{
    return bbox.GetSize().x;
}

void Landscape::SetLandscapeSize(float32 newSize)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    Vector3 newLandscapeSize(newSize, newSize, bbox.GetSize().z);
    SetLandscapeSize(newLandscapeSize);
}

float32 Landscape::GetLandscapeHeight() const
{
    return bbox.GetSize().z;
}

void Landscape::SetLandscapeHeight(float32 newHeight)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    Vector3 newLandscapeSize(bbox.GetSize().x, bbox.GetSize().y, newHeight);
    SetLandscapeSize(newLandscapeSize);
}

void Landscape::SetLandscapeSize(const Vector3& newLandscapeSize)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    if (newLandscapeSize.z < 0.0f || newLandscapeSize.x < 0 || newLandscapeSize.y < 0)
    {
        return;
    }
    if (newLandscapeSize == bbox.GetSize())
    {
        return;
    }
    bbox.Empty();
    bbox.AddPoint(Vector3(-newLandscapeSize.x / 2.f, -newLandscapeSize.y / 2.f, 0.f));
    bbox.AddPoint(Vector3(newLandscapeSize.x / 2.f, newLandscapeSize.y / 2.f, newLandscapeSize.z));
    RebuildLandscape();
}

void Landscape::GetDataNodes(Set<DataNode*>& dataNodes)
{
    for (auto& landscapeLayerRenderer : landscapeLayerRenderers)
    {
        for (uint32 i = 0; i < landscapeLayerRenderer->GetLODCount(); ++i)
        {
            NMaterial* curMaterialNode = landscapeLayerRenderer->GetTerrainLODMaterial(i);
            while (curMaterialNode != nullptr)
            {
                dataNodes.insert(curMaterialNode);
                curMaterialNode = curMaterialNode->GetParent();
            }
        }
    }

    Set<NMaterial*> materialInstances;
    materialInstances.insert(landscapeMaterial);
    for (Vector<DecorationBatch>& batches : decorationBatches)
    {
        for (const DecorationBatch& db : batches)
        {
            if (db.renderBatch != nullptr)
            {
                materialInstances.insert(db.renderBatch->GetMaterial());
            }
        }
    }

    for (NMaterial* m : materialInstances)
    {
        while (m != nullptr)
        {
            dataNodes.insert(m);
            m = m->GetParent();
        }
    }
}

void Landscape::Save(KeyedArchive* archive, SerializationContext* serializationContext)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    BaseObject::SaveObject(archive);

    archive->SetUInt32("ro.debugflags", debugFlags);
    archive->SetUInt32("ro.sOclIndex", staticOcclusionIndex);

    //VI: save only VISIBLE flag for now. May be extended in the future
    archive->SetUInt32("ro.flags", flags & RenderObject::SERIALIZATION_CRITERIA);

    DVASSERT(VT_PAGE_LOD_COUNT == 3);
    for (uint32 layerInd = 0; layerInd < uint32(landscapeLayerRenderers.size()); ++layerInd)
    {
        for (uint32 i = 0; i < VT_PAGE_LOD_COUNT; ++i)
        {
            String matkeyStr = Format("layerInd%d_matname%d", layerInd, i);
            archive->SetUInt64(matkeyStr, landscapeLayerRenderers[layerInd]->GetTerrainLODMaterial(i)->GetNodeID());
        }
    }
    archive->SetUInt64("landscape_matname", landscapeMaterial->GetNodeID());

    //TODO: remove code in future. Need for transition from *.png to *.heightmap
    if (!heightmapPath.IsEqualToExtension(Heightmap::FileExtension()))
    {
        heightmapPath.ReplaceExtension(Heightmap::FileExtension());
    }

    if (heightmap != nullptr)
    {
        heightmap->Save(heightmapPath);
    }
    archive->SetString("hmap", heightmapPath.GetRelativePathname(serializationContext->GetScenePath()));
    archive->SetByteArrayAsType("bbox", bbox);
    archive->SetUInt32("middleLODLevel", GetMiddleLODLevel());
    archive->SetUInt32("macroLODLevel", GetMacroLODLevel());
    archive->SetUInt32("layersCount", layersCount);
    archive->SetUInt32("maxTexturingLevel", maxTexturingLevel);
    archive->SetUInt32("tessellationLevelCount", tessellationLevelCount);
    archive->SetFloat("tessellationHeight", tessellationHeight);

    if (decoration)
    {
        decoration->Save(archive, serializationContext);
    }
}

void Landscape::Load(KeyedArchive* archive, SerializationContext* serializationContext)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    debugFlags = archive->GetUInt32("ro.debugflags", 0);
    staticOcclusionIndex = uint16(archive->GetUInt32("ro.sOclIndex", INVALID_STATIC_OCCLUSION_INDEX));

    layersCount = archive->GetUInt32("layersCount", layersCount);
    DVASSERT(layersCount > 0);
    for (LandscapeLayerRenderer* rend : landscapeLayerRenderers)
        delete rend;
    landscapeLayerRenderers.clear();
    pageManager->ClearPageRenderers();
    decorationPageManager->ClearPageRenderers();
    for (uint32 i = 0; i < layersCount; ++i)
    {
        landscapeLayerRenderers.push_back(new LandscapeLayerRenderer(VT_PAGE_LOD_COUNT));
        pageManager->AddPageRenderer(landscapeLayerRenderers.back());
        decorationPageManager->AddPageRenderer(landscapeLayerRenderers.back());
    }
    pageManager->AddPageRenderer(vtDecalRenderer);
    decorationPageManager->AddPageRenderer(vtDecalRenderer);

    //VI: load only VISIBLE flag for now. May be extended in the future.
    uint32 savedFlags = RenderObject::SERIALIZATION_CRITERIA & archive->GetUInt32("ro.flags", RenderObject::SERIALIZATION_CRITERIA);
    flags = (savedFlags | (flags & ~RenderObject::SERIALIZATION_CRITERIA));

    uint64 matKey = archive->GetUInt64(Format("layerInd%d_matname%d", 0, 0)); // Brand new landscape.

    if (!matKey) // GFX_COMPLETE: Load not so old but not so new material.
    {
        matKey = archive->GetUInt64("matname");

        if (!matKey) //Load from old landscape format: get material from batch0
        {
            uint32 roBatchCount = archive->GetUInt32("ro.batchCount");
            DVASSERT(roBatchCount);
            KeyedArchive* batchesArch = archive->GetArchive("ro.batches");
            DVASSERT(batchesArch);
            KeyedArchive* batchArch = batchesArch->GetArchive(KeyedArchive::GenKeyFromIndex(0));
            DVASSERT(batchArch);

            matKey = batchArch->GetUInt64("rb.nmatname");
        }

        DVASSERT(matKey);
        NMaterial* material = static_cast<NMaterial*>(serializationContext->GetDataBlock(matKey));
        if (material)
        {
            material->AddFlag(NMaterialFlagName::FLAG_LANDSCAPE_VT_PAGE, debugDrawVTPages ? 1 : 0);
            material->AddFlag(NMaterialFlagName::FLAG_DECORATION, 0);
            material->AddFlag(NMaterialFlagName::FLAG_USE_PREVIOUS_LANDSCAPE_LAYER, 0);
            landscapeLayerRenderers[0]->SetTerrainLODMaterial(0, material);
        }
        for (uint32 i = 1; i < VT_PAGE_LOD_COUNT; ++i)
        {
            matKey = archive->GetUInt64(Format("matname%d", i));
            if (matKey)
            {
                material = static_cast<NMaterial*>(serializationContext->GetDataBlock(matKey));
                if (material)
                {
                    material->AddFlag(NMaterialFlagName::FLAG_LANDSCAPE_VT_PAGE, debugDrawVTPages ? 1 : 0);
                    material->AddFlag(NMaterialFlagName::FLAG_DECORATION, 0);
                    landscapeLayerRenderers[0]->SetTerrainLODMaterial(i, material);
                }
            }
        }
    }
    else
    {
        for (uint32 layerInd = 0; layerInd < uint32(landscapeLayerRenderers.size()); ++layerInd)
        {
            for (uint32 i = 0; i < VT_PAGE_LOD_COUNT; ++i)
            {
                matKey = archive->GetUInt64(Format("layerInd%d_matname%d", layerInd, i));
                DVASSERT(matKey);
                NMaterial* material = static_cast<NMaterial*>(serializationContext->GetDataBlock(matKey));
                if (material)
                {
                    material->AddFlag(NMaterialFlagName::FLAG_LANDSCAPE_VT_PAGE, debugDrawVTPages ? 1 : 0);
                    material->AddFlag(NMaterialFlagName::FLAG_DECORATION, 0);
                    if (material->HasLocalFlag(NMaterialFlagName::FLAG_USE_PREVIOUS_LANDSCAPE_LAYER))
                        material->SetFlag(NMaterialFlagName::FLAG_USE_PREVIOUS_LANDSCAPE_LAYER, layerInd != 0);
                    else
                        material->AddFlag(NMaterialFlagName::FLAG_USE_PREVIOUS_LANDSCAPE_LAYER, layerInd != 0);
                    landscapeLayerRenderers[layerInd]->SetTerrainLODMaterial(i, material);
                }
            }
        }
    }

    matKey = archive->GetUInt64("landscape_matname");
    if (matKey)
    {
        landscapeMaterial = SafeRetain(static_cast<NMaterial*>(serializationContext->GetDataBlock(matKey)));
        landscapeMaterial->SetTexture(NMaterialTextureName::TEXTURE_ALBEDO, terrainVTexture->GetLayerTexture(0));
        landscapeMaterial->SetTexture(NMaterialTextureName::TEXTURE_NORMAL, terrainVTexture->GetLayerTexture(1));

        if (landscapeMaterial->HasLocalTexture(TEXTURE_TERRAIN))
            landscapeMaterial->SetTexture(TEXTURE_TERRAIN, terrainVTexture->GetLayerTexture(0));
        else
            landscapeMaterial->AddTexture(TEXTURE_TERRAIN, terrainVTexture->GetLayerTexture(0));

        PrepareMaterial(landscapeMaterial);
    }

    FilePath heightmapPath = serializationContext->GetScenePath() + archive->GetString("hmap");
    AABBox3 loadedBbox = archive->GetByteArrayAsType("bbox", AABBox3());

    SetMiddleLODLevel(archive->GetUInt32("middleLODLevel", 8));
    SetMacroLODLevel(archive->GetUInt32("macroLODLevel", 4));

    maxTexturingLevel = archive->GetUInt32("maxTexturingLevel", maxTexturingLevel);
    tessellationLevelCount = archive->GetUInt32("tessellationLevelCount", tessellationLevelCount);
    tessellationHeight = archive->GetFloat("tessellationHeight", tessellationHeight);

    if (decoration)
    {
        decoration->Load(archive, serializationContext);
    }

    BuildLandscapeFromHeightmapImage(heightmapPath, loadedBbox);
}

Heightmap* Landscape::GetHeightmap()
{
    return heightmap;
}

void Landscape::SetHeightmap(DAVA::Heightmap* height)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    SafeRelease(heightmap);
    heightmap = SafeRetain(height);

    RebuildLandscape();
}

void Landscape::UpdateHeightmap(const Vector<uint16>& data, const Rect2i& rect)
{
    DVASSERT(renderMode == RENDERMODE_INSTANCING_MORPHING);

    heightmap->UpdateHeightmap(data, rect);
    subdivision->UpdateHeightChainData(rect);
    UpdateTextureData(rect);
    UpdateTextures();
}

uint32 Landscape::GetPageMaterialCount(uint32 layerIndex) const
{
    DVASSERT(layerIndex < layersCount);
    return landscapeLayerRenderers[layerIndex]->GetLODCount();
}

NMaterial* Landscape::GetPageMaterials(uint32 layerIndex /*= 0*/, uint32 materialIndex /*= 0*/) const
{
    DVASSERT(layerIndex < layersCount);
    return landscapeLayerRenderers[layerIndex]->GetTerrainLODMaterial(materialIndex);
}

const Vector<LandscapeLayerRenderer*>* Landscape::GetTerrainLayerRenderers() const
{
    return &landscapeLayerRenderers;
}

void Landscape::SetTerrainLayerRenderers(Vector<LandscapeLayerRenderer*>* layerRenderers)
{
}

void Landscape::SetPagesUpdatePerFrame(uint32 count)
{
    maxPagesUpdatePerFrame = count;
}

NMaterial* Landscape::GetLandscapeMaterial()
{
    return landscapeMaterial;
}

void Landscape::SetLandscapeMaterial(NMaterial* material)
{
    SafeRelease(landscapeMaterial);
    landscapeMaterial = SafeRetain(material);

    GetRenderBatch(0)->SetMaterial(landscapeMaterial);
    if (decorationMaterial)
        decorationMaterial->SetParent(landscapeMaterial);
}

RenderObject* Landscape::Clone(RenderObject* newObject)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    if (!newObject)
    {
        DVASSERT(IsPointerToExactClass<Landscape>(this), "Can clone only Landscape");
        newObject = new Landscape();
    }

    Landscape* newLandscape = static_cast<Landscape*>(newObject);

    RefPtr<NMaterial> material(landscapeMaterial->Clone());
    newLandscape->landscapeMaterial = SafeRetain(material.Get());
    newLandscape->layersCount = layersCount;

    for (LandscapeLayerRenderer* r : newLandscape->landscapeLayerRenderers)
        delete r;
    newLandscape->landscapeLayerRenderers.clear();

    for (uint32 layer = 0; layer < layersCount; ++layer)
        newLandscape->landscapeLayerRenderers.push_back(landscapeLayerRenderers[layer]->Clone());

    newLandscape->flags = flags;
    if (decoration)
    {
        newLandscape->decoration = new DecorationData();
        newLandscape->decoration->SetDecorationPath(decoration->GetDecorationPath());
    }

    newLandscape->BuildLandscapeFromHeightmapImage(heightmapPath, bbox);

    return newObject;
}

void Landscape::ResizeIndicesBufferIfNeeded(DAVA::uint32 newSize)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    if (indices.size() < newSize)
    {
        indices.resize(2 * newSize);
    }
};

void Landscape::SetForceMinSubdiv(uint32 level)
{
    subdivision->SetForceMinSubdivision(level);
}

void Landscape::SetUpdatable(bool isUpdatable)
{
    if (updatable != isUpdatable)
    {
        updatable = isUpdatable;
        if (renderMode == RENDERMODE_NO_INSTANCING)
            RebuildLandscape();
    }
}

bool Landscape::IsUpdatable() const
{
    return updatable;
}

void Landscape::SetMaxTexturingLevel(uint32 level)
{
    maxTexturingLevel = level;
    UpdateMaxSubdivisionLevel();
}

uint32 Landscape::GetMaxTexturingLevel() const
{
    return maxTexturingLevel;
}

void Landscape::SetTessellationLevels(uint32 levels)
{
    tessellationLevelCount = levels;
    UpdateMaxSubdivisionLevel();
}

uint32 Landscape::GetTessellationLevels() const
{
    return tessellationLevelCount;
}

void Landscape::SetTessellationHeight(float32 height)
{
    tessellationHeight = height;
}

float32 Landscape::GetTessellationHeight() const
{
    return tessellationHeight;
}

void Landscape::UpdateMaxSubdivisionLevel()
{
    subdivision->SetMaxSubdivisionLevel(Max(heightmapMaxBaseLod, maxTexturingLevel) + (microtessellation ? tessellationLevelCount : 0u));
}

void Landscape::SetMicroTessellation(bool enabled)
{
    microtessellation = enabled;
    landscapeMaterial->SetFlag(NMaterialFlagName::FLAG_LANDSCAPE_MICRO_TESSELLATION, microtessellation ? LANDSCAPE_TESSELLATION_MODE_FLAG : 0);
    UpdateMaxSubdivisionLevel();
}

bool Landscape::GetMicroTessellation() const
{
    return microtessellation;
}

void Landscape::SetDrawWired(bool isWired)
{
    landscapeMaterial->SetFXName(isWired ? NMaterialName::LANDSCAPE_DEBUG : NMaterialName::LANDSCAPE);
}

bool Landscape::IsDrawWired() const
{
    return landscapeMaterial->GetEffectiveFXName() == NMaterialName::LANDSCAPE_DEBUG;
}

void Landscape::SetUseInstancing(bool isUse)
{
    RenderMode newRenderMode = (isUse && rhi::DeviceCaps().isInstancingSupported) ? RENDERMODE_INSTANCING : RENDERMODE_NO_INSTANCING;
    SetRenderMode(newRenderMode);
}

bool Landscape::IsUseInstancing() const
{
    return (renderMode == RENDERMODE_INSTANCING || renderMode == RENDERMODE_INSTANCING_MORPHING);
}

void Landscape::SetUseMorphing(bool useMorph)
{
    RenderMode newRenderMode = useMorph ? RENDERMODE_INSTANCING_MORPHING : RENDERMODE_INSTANCING;
    newRenderMode = rhi::DeviceCaps().isInstancingSupported ? newRenderMode : RENDERMODE_NO_INSTANCING;
    SetRenderMode(newRenderMode);
}

bool Landscape::IsUseMorphing() const
{
    return (renderMode == RENDERMODE_INSTANCING_MORPHING);
}

void Landscape::SetMiddleLODLevel(uint32 level)
{
    if (pageManager)
        pageManager->SetMiddleLODLevel(level);
    if (decorationPageManager)
        decorationPageManager->SetMiddleLODLevel(level);
}

uint32 Landscape::GetMiddleLODLevel() const
{
    return pageManager ? pageManager->GetMiddleLODLevel() : 0u;
}

void Landscape::SetMacroLODLevel(uint32 level)
{
    if (pageManager)
        pageManager->SetMacroLODLevel(level);
    if (decorationPageManager)
        decorationPageManager->SetMacroLODLevel(level);
}

uint32 Landscape::GetMacroLODLevel() const
{
    return pageManager ? pageManager->GetMacroLODLevel() : 0u;
}

void Landscape::SetRenderMode(RenderMode newRenderMode)
{
    //TODO: fix no-instancing render-mode
    DVASSERT(newRenderMode != RENDERMODE_NO_INSTANCING);
    if (newRenderMode == RENDERMODE_NO_INSTANCING)
        return;

    if (renderMode == newRenderMode)
        return;

    renderMode = newRenderMode;

    RebuildLandscape();
    UpdateMaterialFlags();
}

void Landscape::UpdateMaterialFlags()
{
    landscapeMaterial->SetFlag(NMaterialFlagName::FLAG_LANDSCAPE_USE_INSTANCING, (renderMode == RENDERMODE_NO_INSTANCING) ? 0 : 1);
    landscapeMaterial->SetFlag(NMaterialFlagName::FLAG_LANDSCAPE_LOD_MORPHING, (renderMode == RENDERMODE_INSTANCING_MORPHING) ? 1 : 0);
    landscapeMaterial->SetFlag(NMaterialFlagName::FLAG_LANDSCAPE_MICRO_TESSELLATION, microtessellation ? LANDSCAPE_TESSELLATION_MODE_FLAG : 0);
    landscapeMaterial->SetFlag(NMaterialFlagName::FLAG_LANDSCAPE_MORPHING_COLOR, debugDrawMorphing ? 1 : 0);
    landscapeMaterial->SetFlag(NMaterialFlagName::FLAG_LANDSCAPE_TESSELLATION_COLOR, debugDrawTessellationHeight ? 1 : 0);
    landscapeMaterial->SetFlag(NMaterialFlagName::FLAG_LANDSCAPE_PATCHES, debugDrawPatches ? 1 : 0);
}

void Landscape::SetDrawMorphing(bool drawMorph)
{
    if (debugDrawMorphing != drawMorph)
    {
        debugDrawMorphing = drawMorph;
        landscapeMaterial->SetFlag(NMaterialFlagName::FLAG_LANDSCAPE_MORPHING_COLOR, debugDrawMorphing ? 1 : 0);
    }
}

bool Landscape::IsDrawMorphing() const
{
    return debugDrawMorphing;
}

void Landscape::SetDrawTessellationHeight(bool drawTessellationHeight)
{
    debugDrawTessellationHeight = drawTessellationHeight;
    landscapeMaterial->SetFlag(NMaterialFlagName::FLAG_LANDSCAPE_TESSELLATION_COLOR, debugDrawTessellationHeight ? 1 : 0);
}

bool Landscape::IsDrawTessellationHeight() const
{
    return debugDrawTessellationHeight;
}

void Landscape::SetDrawVTPage(bool draw)
{
    if (debugDrawVTPages != draw)
    {
        debugDrawVTPages = draw;

        for (uint32 layer = 0; layer < landscapeLayerRenderers.size(); ++layer)
        {
            for (uint32 i = 0; i < landscapeLayerRenderers[layer]->GetLODCount(); ++i)
            {
                landscapeLayerRenderers[layer]->GetTerrainLODMaterial(i)->SetFlag(NMaterialFlagName::FLAG_LANDSCAPE_VT_PAGE, debugDrawVTPages ? 1 : 0);
            }
        }
        pageManager->Invalidate();
    }
}

bool Landscape::IsDrawVTPage() const
{
    return debugDrawVTPages;
}

void Landscape::SetDrawPatches(bool draw)
{
    if (debugDrawPatches != draw)
    {
        debugDrawPatches = draw;
        landscapeMaterial->SetFlag(NMaterialFlagName::FLAG_LANDSCAPE_PATCHES, debugDrawPatches ? 1 : 0);
    }
}

bool Landscape::IsDrawPatches() const
{
    return debugDrawPatches;
}

void Landscape::SetDrawDecorationLevels(bool draw)
{
    if (debugDrawDecorationLevels != draw)
    {
        debugDrawDecorationLevels = draw;
        if (decorationMaterial)
            decorationMaterial->SetFlag(NMaterialFlagName::FLAG_DECORATION_DRAW_LEVELS, debugDrawDecorationLevels ? 1 : 0);
    }
}

bool Landscape::IsDrawDecorationLevels() const
{
    return debugDrawDecorationLevels;
}

void Landscape::InvalidateAllPages()
{
    pageManager->Invalidate();
    decorationPageManager->Invalidate();
}

void Landscape::InvalidatePages(const Rect& rect, uint32 level, uint32 x, uint32 y)
{
    float32 levelSizef = float32(LandscapeSubdivision::GetLevelSize(level));
    Rect pageRect(x / levelSizef, y / levelSizef, 1.f / levelSizef, 1.f / levelSizef);

    bool wasInvalidated = pageManager->InvalidatePage(level, x, y);
    wasInvalidated |= decorationPageManager->InvalidatePage(level, x, y);
    if (pageRect.RectIntersects(rect) && level < maxTexturingLevel)
    {
        ++level;
        x <<= 1;
        y <<= 1;

        InvalidatePages(rect, level, x, y);
        InvalidatePages(rect, level, x + 1, y);
        InvalidatePages(rect, level, x, y + 1);
        InvalidatePages(rect, level, x + 1, y + 1);
    }
}

void Landscape::UpdatePart(const Rect2i& rect)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    subdivision->UpdateHeightChainData(rect);

    switch (renderMode)
    {
    case RENDERMODE_INSTANCING:
    case RENDERMODE_INSTANCING_MORPHING:
    {
        UpdateTextureData(rect);
        UpdateTextures();
    }
    break;
    case RENDERMODE_NO_INSTANCING:
    {
        DVASSERT(false && "TODO: Landscape::UpdatePart() for non-instancing render");
    }
    break;
    default:
        break;
    }
}

void Landscape::DebugDraw2D(Window*)
{
    if (debugDrawVTexture)
    {
        DVASSERT(terrainVTexture->GetLayersCount() == 2);
        RenderSystem2D::Instance()->DrawTexture(terrainVTexture->GetLayerTexture(0), RenderSystem2D::DEFAULT_2D_TEXTURE_NOBLEND_MATERIAL, Color::White, Rect(5.0f, 5.0f, 512.f, 256.f));
        RenderSystem2D::Instance()->DrawTexture(terrainVTexture->GetLayerTexture(1), RenderSystem2D::DEFAULT_2D_TEXTURE_NOBLEND_MATERIAL, Color::White, Rect(522.f, 5.0f, 512.f, 256.f));

        DVASSERT(decorationVTexture->GetLayersCount() == 1);
        RenderSystem2D::Instance()->DrawTexture(decorationVTexture->GetLayerTexture(0), RenderSystem2D::DEFAULT_2D_TEXTURE_NOBLEND_MATERIAL, Color::White, Rect(5.0f, 266.f, 512.f, 256.f));
    }
}

//////////////////////////////////////////////////////////////////////////
//#decoration

DecorationData* Landscape::GetDecorationData()
{
    return decoration;
}

void Landscape::ReloadDecorationData()
{
    if (decoration != nullptr)
    {
        decoration->ReloadDecoration();
        RebuildDecoration();
    }
}

int32 Landscape::GetTextureCount(eLandscapeTexture textureSemantic) const
{
    int32 result = 0;
    switch (textureSemantic)
    {
    case DAVA::Landscape::HEIGHTMAP_TEXTURE:
        result = 1;
        break;
    case DAVA::Landscape::TANGENT_TEXTURE:
        result = 1;
        break;
    case DAVA::Landscape::TILEMASK_TEXTURE:
        // TODO implement this
        break;
    default:
        break;
    }

    return result;
}

DAVA::Texture* Landscape::GetTexture(eLandscapeTexture textureSemantic, int32 index) const
{
    DAVA::Texture* result = nullptr;
    switch (textureSemantic)
    {
    case DAVA::Landscape::HEIGHTMAP_TEXTURE:
        DVASSERT(index == 0);
        result = heightTexture;
        break;
    case DAVA::Landscape::TANGENT_TEXTURE:
        DVASSERT(index == 0);
        result = normalTexture;
        break;
    case DAVA::Landscape::TILEMASK_TEXTURE:
        // TODO implement this
        break;
    default:
        break;
    }

    return result;
}

void Landscape::SetTexture(eLandscapeTexture textureSemantic, int32 index, Texture* texture)
{
    switch (textureSemantic)
    {
    case DAVA::Landscape::HEIGHTMAP_TEXTURE:
        DVASSERT(index == 0);
        DAVA::SafeRelease(heightTexture);
        heightTexture = DAVA::SafeRetain(texture);
        landscapeMaterial->SetTexture(NMaterialTextureName::TEXTURE_HEIGHTMAP, heightTexture);
        break;
    case DAVA::Landscape::TANGENT_TEXTURE:
        DVASSERT(index == 0);
        DAVA::SafeRelease(normalTexture);
        normalTexture = DAVA::SafeRetain(texture);
        landscapeMaterial->SetTexture(NMaterialTextureName::TEXTURE_TANGENTMAP, normalTexture);
    case DAVA::Landscape::TILEMASK_TEXTURE:
        // TODO implement this
        break;
    default:
        break;
    }
}

float32 Landscape::GetRandomFloat(uint32 layer)
{
    DVASSERT(layer < decoration->GetLayersCount());

    Vector<float32>& buffer = randomBuffer[layer].first;
    uint32& head = randomBuffer[layer].second;

    if (head == uint32(buffer.size()))
        buffer.push_back(float32(Random::Instance()->RandFloat()));

    return buffer[head++];
}

void Landscape::DrawDecorationPatch(const LandscapeSubdivision::SubdivisionPatch* patch)
{
    uint32 decorationLevel = decoration->GetBaseLevel() - patch->level;
    if (decorationLevel >= decoration->GetLevelCount()) //remember that 'decorationLevel' is unsigned
        return;

    if (uint32(decorationInstanceBuffers.size()) <= decorationLevel)
        return;

    uint32 level = patch->level;
    uint32 x = patch->x;
    uint32 y = patch->y;

    decorationInstanceBuffers[decorationLevel].instanceData.emplace_back();
    decorationInstanceBuffers[decorationLevel].instanceCount++;
    InstanceDataDecoration& instanceData = decorationInstanceBuffers[decorationLevel].instanceData.back();

    float32 levelSizef = float32(LandscapeSubdivision::GetLevelSize(level));
    instanceData.patchOffset = Vector2(x / levelSizef, y / levelSizef);
    instanceData.patchScale = 1.f / levelSizef;
    instanceData.decorScale = morphFunc(patch->morphCoeff);

    LandscapePageManager::PageMapping pageMapping = decorationPageManager->GetSuitablePage(level, x, y);
    instanceData.decorPageOffset = pageMapping.uvOffset0;
    instanceData.decorPageScale = pageMapping.uvScale0;

    //////////////////////////////////////////////////////////////////////////
    //Patch flip-rotate

    uint32 minLevelDelta = decoration->GetLevelCount() - decorationLevel - 1;
    uint32 minLevelX = x >> minLevelDelta;
    uint32 minLevelY = y >> minLevelDelta;

    uint32 base = (minLevelX % 8) << 1;
    base = (base < 8) ? base : base - 7;
    uint32 index = (base + minLevelY) % 8;

    static const Array<Vector3, 8> FLIP_ROTATE_DATA = {
        Vector3(0.f, 1.f, 1.f),
        Vector3(1.f, 0.f, 1.f),
        Vector3(0.f, -1.f, 1.f),
        Vector3(-1.f, 0.f, 1.f),
        Vector3(0.f, 1.f, -1.f),
        Vector3(1.f, 0.f, -1.f),
        Vector3(0.f, -1.f, -1.f),
        Vector3(-1.f, 0.f, -1.f)
    };

    instanceData.flipRotate = FLIP_ROTATE_DATA[index];

    //////////////////////////////////////////////////////////////////////////
    //per-patch random rotation

    float32 angle = x * 12.9898f + y * 78.233f + level * 17.3254f;
    SinCosFast(angle, instanceData.randomRotate.x, instanceData.randomRotate.y);
}

void Landscape::ReleaseDecorationRenderData()
{
    for (Vector<DecorationBatch>& batches : decorationBatches)
    {
        for (DecorationBatch& db : batches)
        {
            if (db.renderBatch)
            {
                if (db.renderBatch->vertexBuffer.IsValid())
                    rhi::DeleteVertexBuffer(db.renderBatch->vertexBuffer);

                if (db.renderBatch->indexBuffer.IsValid())
                    rhi::DeleteIndexBuffer(db.renderBatch->indexBuffer);

                db.renderBatch->GetMaterial()->Release();
                SafeRelease(db.renderBatch);
            }
        }
        batches.clear();
    }

    while (GetRenderBatchCount() > 1)
        RemoveRenderBatch(1);

    decorationInstanceBuffers.clear();
}

void Landscape::RebuildDecoration()
{
    ReleaseDecorationRenderData();

    rhi::VertexLayout vLayout;
    vLayout.AddStream(rhi::VDF_PER_VERTEX);
    vLayout.AddElement(rhi::VS_POSITION, 0, rhi::VDT_FLOAT, 3);
    vLayout.AddElement(rhi::VS_TEXCOORD, 0, rhi::VDT_FLOAT, 4); //uv, tint, radius
    vLayout.AddElement(rhi::VS_TEXCOORD, 1, rhi::VDT_FLOAT, 4);
    vLayout.AddElement(rhi::VS_NORMAL, 0, rhi::VDT_FLOAT, 3);
    vLayout.AddElement(rhi::VS_TANGENT, 0, rhi::VDT_FLOAT, 3);
    vLayout.AddElement(rhi::VS_BINORMAL, 0, rhi::VDT_FLOAT, 3);
    vLayout.AddStream(rhi::VDF_PER_INSTANCE);
    vLayout.AddElement(rhi::VS_TEXCOORD, 3, rhi::VDT_FLOAT, 4); //patch position + scale
    vLayout.AddElement(rhi::VS_TEXCOORD, 4, rhi::VDT_FLOAT, 4); //decoration page coords + scale
    vLayout.AddElement(rhi::VS_TEXCOORD, 5, rhi::VDT_FLOAT, 3); //flip-rotate
    vLayout.AddElement(rhi::VS_TEXCOORD, 6, rhi::VDT_FLOAT, 2); //random rotate
    vLayoutDecor = rhi::VertexLayout::UniqueId(vLayout);

    uint32 layersCount = decoration->GetLayersCount();
    uint32 levelCount = decoration->GetLevelCount();

    decorationInstanceBuffers.resize(levelCount);
    decorationBatches.resize(layersCount);

    renderStats.decorationLayerTriangles.resize(layersCount);
    renderStats.decorationLayerItems.resize(layersCount);

    randomBuffer.resize(layersCount);
    for (auto& rand : randomBuffer)
        rand.second = 0;

    if (levelCount == 0 || layersCount == 0)
        return;

    Vector<DecorationVertex> geometryData;
    Vector<uint32> indexData;
    uint32 indexOffset = 0;

    for (uint32 layerIndex = 0; layerIndex < layersCount; ++layerIndex)
    {
        uint32 variationCount = decoration->GetVariationCount(layerIndex);
        NMaterial* layerMaterial = decoration->GetLayerMaterial(layerIndex);

        if (variationCount == 0 || layerMaterial == nullptr)
            continue;

        bool collisionDetection = decoration->GetLayerCollisionDetection(layerIndex);

        Vector<Landscape::DecorationLevelItems> decorationLevelItems = collisionDetection ? GenerateCollisionFreeItems(layerIndex) : GenerateRandomItems(layerIndex);

        bool tintFlag = decoration->GetLayerTint(layerIndex);
        float32 tintHeight = decoration->GetLayerTintHeight(layerIndex);
        decorationBatches[layerIndex].resize(levelCount);

        for (uint32 levelIndex = 0; levelIndex < levelCount; ++levelIndex)
        {
            float32 patchSize = GetLandscapeSize() / (1 << (decoration->GetBaseLevel() - levelIndex));

            geometryData.clear();
            indexData.clear();
            indexOffset = 0u;
            decorationBatches[layerIndex][levelIndex].itemsCount = 0u;

            for (uint32 v = 0; v < variationCount; ++v)
            {
                const DecorationVariationItems& variationItems = decorationLevelItems[levelIndex][v];
                uint32 variationItemCount = uint32(variationItems.size());

                float32 scaleMin = decoration->GetVariationScaleMin(layerIndex, v);
                float32 scaleMax = decoration->GetVariationScaleMax(layerIndex, v);
                float32 pitchMax = decoration->GetVariationPitchMax(layerIndex, v);
                float32 collisionRadius = decoration->GetVariationCollisionRadius(layerIndex, v);

                PolygonGroup* geometry = decoration->GetVariationGeometry(layerIndex, v);
                geometryData.reserve(geometryData.size() + geometry->vertexCount * variationItemCount);
                indexData.reserve(indexData.size() + geometry->indexCount * variationItemCount);
                for (const DecorationItem& item : variationItems)
                {
                    float32 scale = scaleMin + (scaleMax - scaleMin) * GetRandomFloat(layerIndex);

                    Vector2 pivot;
                    float32 itemCircleRadius = 0.f;
                    if (collisionDetection)
                    {
                        float32 relativeCollisionRadius = collisionRadius * scale / patchSize;
                        itemCircleRadius = Max(item.randomCircleRadius / float32(1 << levelIndex) - relativeCollisionRadius, 0.f);

                        float32 randomAngle = GetRandomFloat(layerIndex) * PI_2;
                        float32 randomRadius = std::sqrtf(GetRandomFloat(layerIndex)) * itemCircleRadius;
                        pivot = item.randomCircleCenter + Vector2(std::cosf(randomAngle), std::sinf(randomAngle)) * randomRadius;
                    }
                    else
                    {
                        pivot = item.randomCircleCenter;
                    }

                    float32 pitchAxisAngle = GetRandomFloat(layerIndex) * PI_2;
                    Vector3 pitchAxis(std::cosf(pitchAxisAngle), std::sinf(pitchAxisAngle), 0.f);

                    float32 pitch = DegToRad(pitchMax * GetRandomFloat(layerIndex));
                    float32 rotation = GetRandomFloat(layerIndex) * PI_2;

                    if (!decoration->GetVariationEnabled(layerIndex, v))
                        continue;

                    Matrix4 rotationMx = Matrix4::MakeRotation(Vector3(0.0f, 0.0f, 1.0f), rotation) * Matrix4::MakeRotation(pitchAxis, pitch);
                    Matrix4 invTMx = rotationMx;
                    invTMx.Inverse();
                    invTMx.Transpose();
                    Matrix3 normalMx3 = invTMx;
                    for (int32 vi = 0; vi < geometry->vertexCount; ++vi)
                    {
                        DecorationVertex vx;

                        geometry->GetCoord(vi, vx.position);
                        geometry->GetTexcoord(0, vi, vx.texCoord);
                        geometry->GetNormal(vi, vx.normal);
                        geometry->GetTangent(vi, vx.tangent);
                        geometry->GetBinormal(vi, vx.binormal);

                        vx.position = vx.position * rotationMx * scale;
                        vx.normal = vx.normal * normalMx3;
                        vx.tangent = vx.tangent * normalMx3;
                        vx.binormal = vx.binormal * normalMx3;
                        vx.pivot = pivot;
                        vx.circle = item.randomCircleCenter;
                        vx.radius = itemCircleRadius;

                        float32 tintValue = 0.f;
                        if (tintHeight > EPSILON)
                            tintValue = 1.f - Clamp(vx.position.z / tintHeight, 0.f, 1.f);

                        vx.tint = tintValue * (tintFlag ? 1.f : 0.f);

                        geometryData.push_back(vx);
                    }

                    int32 index;
                    for (int32 ii = 0; ii < geometry->GetIndexCount(); ++ii)
                    {
                        geometry->GetIndex(ii, index);
                        indexData.push_back(indexOffset + uint32(index));
                    }

                    indexOffset += geometry->vertexCount;
                }

                if (decoration->GetVariationEnabled(layerIndex, v))
                    decorationBatches[layerIndex][levelIndex].itemsCount += variationItemCount;
            }

            if (!geometryData.empty() && !indexData.empty())
            {
                rhi::VertexBuffer::Descriptor vbufferDesc;
                vbufferDesc.initialData = geometryData.data();
                vbufferDesc.needRestore = true;
                vbufferDesc.size = uint32(geometryData.size() * sizeof(DecorationVertex));
                vbufferDesc.usage = rhi::USAGE_STATICDRAW;
                rhi::HVertexBuffer vBuffer = rhi::CreateVertexBuffer(vbufferDesc);

                rhi::IndexBuffer::Descriptor ibufferDesc;
                ibufferDesc.indexSize = rhi::INDEX_SIZE_32BIT;
                ibufferDesc.initialData = indexData.data();
                ibufferDesc.needRestore = true;
                ibufferDesc.size = uint32(indexData.size() * sizeof(uint32));
                ibufferDesc.usage = rhi::USAGE_STATICDRAW;
                rhi::HIndexBuffer iBuffer = rhi::CreateIndexBuffer(ibufferDesc);

                static const Vector4 levelColor[6] = {
                    Vector4(1.f, 0.f, 0.f, 1.f),
                    Vector4(0.f, 1.f, 0.f, 1.f),
                    Vector4(0.f, 0.f, 1.f, 1.f),
                    Vector4(1.f, 0.f, 1.f, 1.f),
                    Vector4(0.f, 1.f, 1.f, 1.f),
                    Vector4(1.f, 1.f, 0.f, 1.f)
                };

                uint8 layerMask = decoration->GetLayerMask(layerIndex);
                Vector4 layerMaskProperty = Vector4(
                ((layerMask & DecorationData::LAYER_MASK_CHANNEL_R) ? 1.f : 0.f),
                ((layerMask & DecorationData::LAYER_MASK_CHANNEL_G) ? 1.f : 0.f),
                ((layerMask & DecorationData::LAYER_MASK_CHANNEL_B) ? 1.f : 0.f),
                ((layerMask & DecorationData::LAYER_MASK_CHANNEL_A) ? 1.f : 0.f)
                );

                float32 layerOrientValue = decoration->GetLayerOrientValue(layerIndex);

                NMaterial* material = layerMaterial->Clone();
                material->SetFXName(decoration->GetLayerCullface(layerIndex) ? NMaterialName::DECORATION_CULLFACE : NMaterialName::DECORATION);
                //material->AddProperty(FastName("baseColorScale"), levelColor[Min(levelIndex, 5u)].data, rhi::ShaderProp::TYPE_FLOAT4);
                material->AddProperty(PARAM_DECORATION_DECORATION_MASK, layerMaskProperty.data, rhi::ShaderProp::TYPE_FLOAT4);
                material->AddProperty(PARAM_DECORATION_ORIENT_VALUE, &layerOrientValue, rhi::ShaderProp::TYPE_FLOAT1);
                material->AddFlag(NMaterialFlagName::FLAG_VERTEX_COLOR, tintFlag ? 2 : 0);
                material->AddFlag(FLAG_DECORATION_ORIENT_ON_LANDSCAPE, decoration->GetLayerOrientOnLandscape(layerIndex) ? 1 : 0);
                material->AddFlag(FLAG_DECORATION_GPU_RANDOMIZATION, decoration->GetLayerCollisionDetection(layerIndex) ? 2 : 1);
                material->SetParent(decorationMaterial);
                material->SetRuntime(true);

                RenderBatch* batch = new RenderBatch();
                batch->SetMaterial(material);
                batch->vertexBuffer = vBuffer;
                batch->indexBuffer = iBuffer;
                batch->primitiveType = rhi::PRIMITIVE_TRIANGLELIST;
                batch->indexCount = uint32(indexData.size());
                batch->vertexCount = uint32(geometryData.size());
                batch->vertexLayoutId = vLayoutDecor;

                decorationBatches[layerIndex][levelIndex].renderBatch = batch;
                AddRenderBatch(batch);
            }
        }
    }
}

Vector<Landscape::DecorationLevelItems> Landscape::GenerateCollisionFreeItems(uint32 layerIndex)
{
    UnorderedMap<uint32, DecorationCollisionGroupData> collisionGroupInfo;

    float32 baseLevelPatchSize = GetLandscapeSize() / (1 << decoration->GetBaseLevel());

    uint32 variationCount = decoration->GetVariationCount(layerIndex);
    for (uint32 v = 0; v < variationCount; ++v)
    {
        DecorationCollisionGroupData& collisionGroup = collisionGroupInfo[decoration->GetVariationCollisionGroup(layerIndex, v)];

        collisionGroup.variations.emplace_back(v);
        collisionGroup.maxDensity += decoration->GetVariationDensity(layerIndex, v);
    }

    for (auto& it : collisionGroupInfo)
    {
        uint32 collisionGroupID = it.first;
        DecorationCollisionGroupData& collisionGroup = it.second;
        uint32 itemCount = uint32(collisionGroup.maxDensity * baseLevelPatchSize * baseLevelPatchSize);

        if (itemCount == 0)
            continue;

        if (itemCount <= PackedCircles::CIRCLES_PACKINGS_COUNT)
        {
            uint32 packedCirclesCoordsBaseIndex = PackedCircles::GetCirclesCoordsBaseIndex(itemCount);

            auto circlesCoordsBase = PackedCircles::CIRCLES_PACKINGS_COORDS.begin() + packedCirclesCoordsBaseIndex;
            collisionGroup.itemCoords.assign(circlesCoordsBase, circlesCoordsBase + itemCount);
            collisionGroup.circlesRadius = PackedCircles::CIRCLES_PACKINGS_RADIUSES[itemCount - 1];
        }
        else
        {
            uint32 squareGridSize = uint32(std::roundf(std::sqrtf(float32(itemCount)) + 0.5f - 1.0e-6f));
            collisionGroup.itemCoords.resize(squareGridSize * squareGridSize);
            for (uint32 i = 0; i < squareGridSize; ++i)
            {
                for (uint32 j = 0; j < squareGridSize; ++j)
                {
                    Vector2 quadCoord = Vector2(i + 0.5f, j + 0.5f) / float32(squareGridSize);
                    collisionGroup.itemCoords[i * squareGridSize + j] = quadCoord - Vector2(.5f, .5f);
                }
            }

            std::shuffle(collisionGroup.itemCoords.begin(), collisionGroup.itemCoords.end(), std::default_random_engine(collisionGroupID + 273));
            collisionGroup.itemCoords.resize(itemCount);

            collisionGroup.circlesRadius = 0.5f / squareGridSize;
        }
    }

    uint32 levelCount = decoration->GetLevelCount();
    Vector<Landscape::DecorationLevelItems> levelsItems(levelCount);
    for (uint32 levelIndex = 0; levelIndex < levelCount; ++levelIndex)
    {
        DecorationLevelItems& levelItems = levelsItems[levelIndex];
        levelItems.resize(variationCount);

        for (auto& it : collisionGroupInfo)
        {
            uint32 collisionGroupID = it.first;
            DecorationCollisionGroupData& groupData = it.second;

            Vector<Vector2>& groupItemCoords = groupData.itemCoords;

            std::shuffle(groupItemCoords.begin(), groupItemCoords.end(), std::default_random_engine((collisionGroupID + 289) << levelIndex));

            for (uint32 variation : groupData.variations)
            {
                float32 varDensity = decoration->GetVariationDensity(layerIndex, variation);
                float32 levelDensity0 = decoration->GetLevelDensity(layerIndex, variation, levelIndex);
                float32 levelDensity1 = ((levelIndex + 1) == levelCount) ? 0.f : decoration->GetLevelDensity(layerIndex, variation, levelIndex + 1);
                float32 relativeDensity = levelDensity0 - levelDensity1;

                float32 patchSize = GetLandscapeSize() / (1 << (decoration->GetBaseLevel() - levelIndex));
                uint32 variationItemsCount = Min(uint32(std::roundf(varDensity * Clamp(relativeDensity, 0.f, 1.f) * patchSize * patchSize)), uint32(groupItemCoords.size()));

                DecorationVariationItems& variationItems = levelItems[variation];
                variationItems.reserve(variationItems.size() + variationItemsCount);
                for (uint32 i = 0; i < variationItemsCount; ++i)
                {
                    variationItems.emplace_back(DecorationItem(groupItemCoords.back() + Vector2(0.5f, 0.5f), groupData.circlesRadius));
                    groupItemCoords.pop_back();
                }
            }

            if (levelIndex == (levelCount - 1) && !groupItemCoords.empty())
            {
                uint32 itemsLeft = uint32(groupItemCoords.size());
                for (uint32 variation : groupData.variations)
                {
                    float32 varDensity = decoration->GetVariationDensity(layerIndex, variation);
                    uint32 variationItemsCount = Min(uint32(std::roundf(itemsLeft * varDensity / groupData.maxDensity)), uint32(groupItemCoords.size()));

                    DecorationVariationItems& variationItems = levelItems[variation];
                    variationItems.reserve(variationItems.size() + variationItemsCount);
                    for (uint32 i = 0; i < variationItemsCount; ++i)
                    {
                        variationItems.emplace_back(DecorationItem(groupItemCoords.back() + Vector2(0.5f, 0.5f), groupData.circlesRadius));
                        groupItemCoords.pop_back();
                    }
                }
            }

            Vector<Vector2> extendedItemCoords;
            extendedItemCoords.reserve(groupData.itemCoords.size() * 4);
            for (const Vector2& coord : groupData.itemCoords)
            {
                Vector2 scaledCoord = coord * 0.5f;
                extendedItemCoords.emplace_back(scaledCoord + Vector2(0.25f, 0.25f));
                extendedItemCoords.emplace_back(scaledCoord + Vector2(-0.25f, 0.25f));
                extendedItemCoords.emplace_back(scaledCoord + Vector2(0.25f, -0.25f));
                extendedItemCoords.emplace_back(scaledCoord + Vector2(-0.25f, -0.25f));
            }

            groupData.itemCoords = std::move(extendedItemCoords);
        }
    }

    return levelsItems;
}

Vector<Landscape::DecorationLevelItems> Landscape::GenerateRandomItems(uint32 layerIndex)
{
    uint32 levelCount = decoration->GetLevelCount();
    uint32 variationCount = decoration->GetVariationCount(layerIndex);

    Vector<DecorationLevelItems> levelsItems(levelCount);
    for (uint32 levelIndex = 0; levelIndex < levelCount; ++levelIndex)
    {
        DecorationLevelItems& levelItems = levelsItems[levelIndex];
        levelItems.resize(variationCount);

        for (uint32 variation = 0; variation < variationCount; ++variation)
        {
            float32 varDensity = decoration->GetVariationDensity(layerIndex, variation);
            float32 levelDensity0 = decoration->GetLevelDensity(layerIndex, variation, levelIndex);
            float32 levelDensity1 = ((levelIndex + 1) == levelCount) ? 0.f : decoration->GetLevelDensity(layerIndex, variation, levelIndex + 1);
            float32 relativeDensity = levelDensity0 - levelDensity1;

            float32 patchSize = GetLandscapeSize() / (1 << (decoration->GetBaseLevel() - levelIndex));
            uint32 variationItemsCount = uint32(std::roundf(varDensity * Clamp(relativeDensity, 0.f, 1.f) * patchSize * patchSize));

            DecorationVariationItems& items = levelsItems[levelIndex][variation];
            items.reserve(items.size() + variationItemsCount);
            for (uint32 i = 0; i < variationItemsCount; ++i)
            {
                items.emplace_back(DecorationItem(Vector2(GetRandomFloat(layerIndex), GetRandomFloat(layerIndex)), 0.f));
            }
        }
    }

    return levelsItems;
}

void Landscape::RecursiveRayTrace(uint32 level, uint32 x, uint32 y, const Ray3& rayInObjectSpace, float32& resultT)
{
#if 0
    const LandscapeSubdivision::SubdivisionPatchInfo& subdivPatchInfo = subdivision->GetPatchInfo(level, x, y);
    const LandscapeSubdivision::PatchQuadInfo& patchQuadInfo = subdivision->GetPatchQuadInfo(level, x, y);

    float32 tMin, tMax;
    bool currentPatchIntersects = Intersection::RayBox(rayInObjectSpace, patchQuadInfo.bbox, tMin, tMax);

    if (currentPatchIntersects)
    {
        if (level == subdivision->GetLevelCount() - 1)
        {
            int32 hmSize = heightmap->Size();
            uint32 patchSize = hmSize >> level;

            uint32 xx = x * patchSize;
            uint32 yy = y * patchSize;

            float32 localResult = FLT_MAX;
            /*int32 collisionCellX2 = -1;
            int32 collisionCellY2 = -1;

            float32 localResult2 = FLT_MAX;
            // Bruteforce algorithm
            {
            for (uint32 y0 = yy; y0 < yy + patchSize; y0 ++)
            {
            for (uint32 x0 = xx; x0 < xx + patchSize; x0 ++)
            {
            uint32 x1 = x0 + 1;
            uint32 y1 = y0 + 1;

            Vector3 p00 = heightmap->GetPoint(x0, y0, bbox);
            Vector3 p01 = heightmap->GetPoint(x0, y1, bbox);
            Vector3 p10 = heightmap->GetPoint(x1, y0, bbox);
            Vector3 p11 = heightmap->GetPoint(x1, y1, bbox);

            float32 t1, t2;
            if (Intersection::RayTriangle(rayInObjectSpace, p00, p01, p11, t1))
            {
            if (t1 < localResult2)
            {
            localResult2 = t1;
            collisionCellX2 = x0;
            collisionCellY2 = y0;
            }
            }

            if (Intersection::RayTriangle(rayInObjectSpace, p00, p11, p10, t2))
            {
            if (t2 < localResult2)
            {
            localResult2 = t2;
            collisionCellX2 = x0;
            collisionCellY2 = y0;

            }
            }
            }
            }
            }

            //Logger::Debug("bf res: %d %d" , collisionCellX2, collisionCellY2);

            //DVASSERT(FLOAT_EQUAL(localResult2, localResult));
            resultT = Min(resultT, localResult2);*/
            float32 localResult3 = FLT_MAX;
            {
                Vector3 grid00 = heightmap->GetPoint(xx, yy, bbox);
                Vector3 gridCellSize = heightmap->GetPoint(xx + 1, yy + 1, bbox) - grid00;

                auto GetPosToVoxel = [patchSize, grid00, gridCellSize](const Vector3 & point, int32 axis)
                {
                    int32 pos = static_cast<int32>((point.data[axis] - grid00.data[axis]) / (float32)gridCellSize.data[axis]);
                    return Clamp(pos, 0, (int32)patchSize - 1);
                };

                auto GetVoxelToPos = [patchSize, grid00, gridCellSize](int32 voxel, int32 axis)
                {
                    float32 pos = (float32)(voxel)* gridCellSize.data[axis] + grid00.data[axis];
                    return pos;
                };



                Vector3 patchIntersect = rayInObjectSpace.origin + rayInObjectSpace.direction * tMin;


                float32 width[3] = { gridCellSize.x, gridCellSize.y, gridCellSize.z };


                // Set up 3D DDA for ray
                float32 nextCrossingT[3], deltaT[3];
                int32 step[3], out[3], pos[3];
                for (int axis = 0; axis < 2; ++axis)
                {
                    // Compute current voxel for axis
                    pos[axis] = GetPosToVoxel(patchIntersect, axis);
                    if (rayInObjectSpace.direction.data[axis] >= 0)
                    {
                        // Handle ray with positive direction for voxel stepping
                        nextCrossingT[axis] = tMin + (GetVoxelToPos(pos[axis] + 1, axis) - patchIntersect.data[axis]) / rayInObjectSpace.direction.data[axis];
                        deltaT[axis] = width[axis] / rayInObjectSpace.direction.data[axis];
                        step[axis] = 1;
                        out[axis] = patchSize;
                    }
                    else
                    {
                        // Handle ray with negative direction for voxel stepping
                        nextCrossingT[axis] = tMin + (GetVoxelToPos(pos[axis], axis) - patchIntersect.data[axis]) / rayInObjectSpace.direction.data[axis];
                        deltaT[axis] = -width[axis] / rayInObjectSpace.direction.data[axis];
                        step[axis] = -1;
                        out[axis] = -1;
                    }
                }

                bool hitSomething = false;
                for (;;)
                {
                    //Logger::Debug("na: %d %d", xx + pos[0], yy + pos[1]);
                    // Check for intersection in current voxel and advance to next
                    Vector3 p00 = heightmap->GetPoint(xx + pos[0], yy + pos[1], bbox);
                    Vector3 p01 = heightmap->GetPoint(xx + pos[0], yy + pos[1] + 1, bbox);
                    Vector3 p10 = heightmap->GetPoint(xx + pos[0] + 1, yy + pos[1], bbox);
                    Vector3 p11 = heightmap->GetPoint(xx + pos[0] + 1, yy + pos[1] + 1, bbox);

                    float32 t1, t2;
                    if (Intersection::RayTriangle(rayInObjectSpace, p00, p01, p11, t1))
                    {
                        if (t1 < localResult3)
                        {
                            localResult3 = t1;
                        }
                    }

                    if (Intersection::RayTriangle(rayInObjectSpace, p00, p11, p10, t2))
                    {
                        if (t2 < localResult3)
                        {
                            localResult3 = t2;
                        }
                    }


                    // Advance to next voxel

                    // Find _stepAxis_ for stepping to next voxel
                    /*int bits = (   (nextCrossingT[0] < nextCrossingT[1]) << 2)  // X < Y
                    + ((nextCrossingT[0] < nextCrossingT[2]) << 1)  // X < Z
                    + ((nextCrossingT[1] < nextCrossingT[2]));      // Y < Z
                    //    0 - ZYX
                    //    7 - XYZ


                    const int cmpToAxis[8] = { 2, 1, 2, 1, 2, 2, 0, 0 };
                    int stepAxis = cmpToAxis[bits];
                    */


                    int32 stepAxis = 0;
                    if (nextCrossingT[0] < nextCrossingT[1])stepAxis = 0;
                    else stepAxis = 1;

                    if (tMax < nextCrossingT[stepAxis])
                        break;
                    pos[stepAxis] += step[stepAxis];
                    if (pos[stepAxis] == out[stepAxis])
                        break;
                    nextCrossingT[stepAxis] += deltaT[stepAxis];
                }
            }
            //if (!FLOAT_EQUAL(localResult2, localResult3))
            //            {
            //                int i = 0;
            //                Logger::Error("DDA Algorithm mistake");
            //            }

            resultT = Min(resultT, localResult3);


        } // End of intersection code


          // If level is not final go deeper
        if ((level < subdivision->GetLevelCount() - 1))
        {
            uint32 x2 = x << 1;
            uint32 y2 = y << 1;

            RecursiveRayTrace(level + 1, x2 + 0, y2 + 0, rayInObjectSpace, resultT);
            RecursiveRayTrace(level + 1, x2 + 1, y2 + 0, rayInObjectSpace, resultT);
            RecursiveRayTrace(level + 1, x2 + 0, y2 + 1, rayInObjectSpace, resultT);
            RecursiveRayTrace(level + 1, x2 + 1, y2 + 1, rayInObjectSpace, resultT);
        }
    }
#endif
}

bool Landscape::RayTrace(const Ray3& rayInObjectSpace, float32& resultT)
{
    float32 localResultT = FLOAT_MAX;
    RecursiveRayTrace(0, 0, 0, rayInObjectSpace, localResultT);

    if (localResultT < FLOAT_MAX)
    {
        resultT = localResultT;
        return true;
    }
    return false;
}

uint32 Landscape::GetLayersCount() const
{
    return layersCount;
}

void Landscape::SetLayersCount(uint32 count)
{
    if (count == layersCount || count == 0)
        return;

    int32 countDiff = count - uint32(landscapeLayerRenderers.size());
    if (countDiff > 0)
    {
        for (int32 i = 0; i < countDiff; ++i)
            landscapeLayerRenderers.push_back(new LandscapeLayerRenderer(VT_PAGE_LOD_COUNT));
    }
    else
    {
        auto it = landscapeLayerRenderers.begin() + count;
        while (it != landscapeLayerRenderers.end())
        {
            delete *it;
            std::advance(it, 1);
        }
        landscapeLayerRenderers.erase(landscapeLayerRenderers.begin() + count, landscapeLayerRenderers.end());
    }
    pageManager->ClearPageRenderers();
    decorationPageManager->ClearPageRenderers();
    for (auto& rend : landscapeLayerRenderers)
    {
        pageManager->AddPageRenderer(rend);
        decorationPageManager->AddPageRenderer(rend);
    }

    pageManager->AddPageRenderer(vtDecalRenderer);
    decorationPageManager->AddPageRenderer(vtDecalRenderer);
    RebuildLandscape();

    layersCount = count;
}
}
