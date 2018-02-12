#include "LandscapeLayerRenderer.h"
#include "Debug/ProfilerMarkerNames.h"
#include "Debug/ProfilerGPU.h"
#include "Render/Material/NMaterial.h"
#include "Render/Material/NMaterialNames.h"
#include "Render/DynamicBufferAllocator.h"
#include "Render/Highlevel/LandscapeSubdivision.h"
#include "Render/Highlevel/RenderPassNames.h"
#include "Render/Highlevel/LandscapePageRenderer.h"
#include "Render/VirtualTexture.h"
#include "Reflection/Reflection.h"
#include "Reflection/ReflectedMeta.h"
#include "Reflection/ReflectionRegistrator.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(LandscapeLayerRenderer)
{
    ReflectionRegistrator<LandscapeLayerRenderer>::Begin()
    .Field("microPageMaterial", &LandscapeLayerRenderer::GetMicroMaterialRefl, &LandscapeLayerRenderer::SetMicroMaterialRefl)[M::ReadOnly()]
    .Field("middlePageMaterial", &LandscapeLayerRenderer::GetMiddleMaterialRefl, &LandscapeLayerRenderer::SetMiddleMaterialRefl)[M::ReadOnly()]
    .Field("macroMaterial", &LandscapeLayerRenderer::GetMacroMaterialRefl, &LandscapeLayerRenderer::SetMacroMaterialRefl)[M::ReadOnly()]
    .End();
}

LandscapeLayerRenderer::LandscapeLayerRenderer(uint32 lodCount)
{
    terrainMaterials.resize(lodCount);
    decorationMaterials.resize(lodCount);
}

LandscapeLayerRenderer::~LandscapeLayerRenderer()
{
    for (NMaterial*& m : terrainMaterials)
        SafeRelease(m);
    terrainMaterials.clear();

    for (NMaterial*& m : decorationMaterials)
        SafeRelease(m);
    decorationMaterials.clear();
}

LandscapeLayerRenderer* LandscapeLayerRenderer::Clone() const
{
    LandscapeLayerRenderer* newRend = new LandscapeLayerRenderer(GetLODCount());
    for (uint32 i = 0; i < uint32(terrainMaterials.size()); ++i)
        newRend->terrainMaterials.push_back(terrainMaterials[i]->Clone());
    for (uint32 i = 0; i < uint32(decorationMaterials.size()); ++i)
        newRend->decorationMaterials.push_back(decorationMaterials[i]->Clone());
    return newRend;
}

bool LandscapeLayerRenderer::RenderPage(const PageRenderParams& params)
{
    struct QuadVertex
    {
        Vector3 pos;
        Vector2 uv;
    };

    static uint32 layout = 0;
    static rhi::Packet quadPacket = {};
    if (layout == 0)
    {
        rhi::VertexLayout vLayout;
        vLayout.AddElement(rhi::VS_POSITION, 0, rhi::VDT_FLOAT, 3);
        vLayout.AddElement(rhi::VS_TEXCOORD, 0, rhi::VDT_FLOAT, 2);
        layout = rhi::VertexLayout::UniqueId(vLayout);

        quadPacket.vertexStreamCount = 1;
        quadPacket.vertexCount = 4;
        quadPacket.primitiveType = rhi::PRIMITIVE_TRIANGLELIST;
        quadPacket.primitiveCount = 2;
        quadPacket.vertexLayoutUID = layout;
    }

    DynamicBufferAllocator::AllocResultVB vb = DynamicBufferAllocator::AllocateVertexBuffer(sizeof(QuadVertex), 4);
    quadPacket.vertexStream[0] = vb.buffer;
    quadPacket.baseVertex = vb.baseVertex;
    quadPacket.vertexCount = vb.allocatedVertices;
    quadPacket.indexBuffer = DynamicBufferAllocator::AllocateQuadListIndexBuffer(1);

    DVASSERT(params.lod < GetLODCount());

    DVASSERT(params.component < eLandscapeComponent::COMPONENT_COUNT);
    NMaterial* material = (params.component == eLandscapeComponent::COMPONENT_TERRAIN) ? terrainMaterials[params.lod] : decorationMaterials[params.lod];

    for (uint32 i = 0; i < static_cast<uint32>(params.pageSrc.size()); ++i)
        Renderer::GetDynamicBindings().SetDynamicTexture(static_cast<DynamicBindings::eTextureSemantic>(DynamicBindings::DYNAMIC_TEXTURE_SRC_0 + i), params.pageSrc[i]->handle);

    if (material->PreBuildMaterial(PASS_FORWARD))
        material->BindParams(quadPacket);

    // float fPageSize = static_cast<float>(vTexture->GetPageSize());
    Vector3 offset(0.0f, 0.0f, 0.0f); // GFX_COMPLETE
    // Deal with half-pixel offset on DX9
    // = rhi::DeviceCaps().isCenterPixelMapping ? Vector3(1.0f / fPageSize, 1.0f / fPageSize, 0.0f) : Vector3(0.0f, 0.0f, 0.0f);

    Vector2 uv0 = params.relativeCoord0;
    Vector2 uv1 = params.relativeCoord1;
    uint32 layerCount = uint32(params.pageDst.size());

    QuadVertex* quadData = reinterpret_cast<QuadVertex*>(vb.data);
    quadData[0].pos = Vector3(-1.0f, -1.0f, 0.0f) + offset;
    quadData[1].pos = Vector3(-1.0f, +1.0f, 0.0f) + offset;
    quadData[2].pos = Vector3(+1.0f, -1.0f, 0.0f) + offset;
    quadData[3].pos = Vector3(+1.0f, +1.0f, 0.0f) + offset;
    quadData[0].uv = Vector2(uv0.x, uv0.y);
    quadData[1].uv = Vector2(uv0.x, uv1.y);
    quadData[2].uv = Vector2(uv1.x, uv0.y);
    quadData[3].uv = Vector2(uv1.x, uv1.y);

    rhi::RenderPassConfig passConfig;
    passConfig.name = ProfilerGPUMarkerName::LANDSCAPE_PAGE_UPDATE;
    passConfig.priority = PRIORITY_SERVICE_3D + 20;
    for (uint32 l = 0; l < layerCount; ++l)
    {
        passConfig.colorBuffer[l].texture = params.pageDst[l]->handle;
        passConfig.colorBuffer[l].loadAction = rhi::LOADACTION_NONE;
        passConfig.colorBuffer[l].storeAction = rhi::STOREACTION_STORE;
    }
    passConfig.depthStencilBuffer.texture = rhi::InvalidHandle;
    passConfig.viewport.x = 0;
    passConfig.viewport.y = 0;
    passConfig.viewport.width = params.pageSize;
    passConfig.viewport.height = params.pageSize;

    DAVA_PROFILER_GPU_RENDER_PASS(passConfig, ProfilerGPUMarkerName::LANDSCAPE_PAGE_UPDATE);
    rhi::HPacketList packetList;
    rhi::HRenderPass pass = rhi::AllocateRenderPass(passConfig, 1, &packetList);
    rhi::BeginRenderPass(pass);
    rhi::BeginPacketList(packetList);
    rhi::AddPacket(packetList, quadPacket);
    rhi::EndPacketList(packetList);
    rhi::EndRenderPass(pass);

    return true;
}

DAVA::NMaterial* LandscapeLayerRenderer::GetMicroMaterialRefl() const
{
    DVASSERT(terrainMaterials.size() > 0);
    return terrainMaterials[0];
}

void LandscapeLayerRenderer::SetMicroMaterialRefl(NMaterial* material)
{
}

DAVA::NMaterial* LandscapeLayerRenderer::GetMiddleMaterialRefl() const
{
    DVASSERT(terrainMaterials.size() > 1);
    return terrainMaterials[1];
}

void LandscapeLayerRenderer::SetMiddleMaterialRefl(NMaterial* material)
{
}

DAVA::NMaterial* LandscapeLayerRenderer::GetMacroMaterialRefl() const
{
    DVASSERT(terrainMaterials.size() > 2);
    return terrainMaterials[2];
}

void LandscapeLayerRenderer::SetMacroMaterialRefl(NMaterial* material)
{
}

}; //ns DAVA
