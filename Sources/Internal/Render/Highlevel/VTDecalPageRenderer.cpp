#include "VTDecalPageRenderer.h"
#include "Debug/ProfilerMarkerNames.h"
#include "Debug/ProfilerGPU.h"
#include "Math/Vector.h"
#include "Render/DynamicBufferAllocator.h"
#include "Render/Highlevel/DecalRenderObject.h"
#include "Render/Highlevel/RenderHierarchy.h"
#include "Render/Highlevel/RenderPassNames.h"
#include "Render/Highlevel/VTDecalManager.h"
#include "Render/Material/NMaterialNames.h"
#include "Render/Highlevel/RenderPassNames.h"
#include "Render/RhiUtils.h"

namespace DAVA
{
VTDecalPageRenderer::VTDecalPageRenderer(bool useFetch_)
    : useFetch(useFetch_)
{
    rhi::VertexLayout vtDecalLayout;
    vtDecalLayout.AddElement(rhi::VS_POSITION, 0, rhi::VDT_FLOAT, 2); //vertex position	in decal space
    vtDecalLayout.AddElement(rhi::VS_TEXCOORD, 0, rhi::VDT_FLOAT, 3); //uv, val
    vtDecalLayout.AddElement(rhi::VS_TEXCOORD, 2, rhi::VDT_FLOAT, 4); //t, b

    vtDecalsPacket.vertexStreamCount = 1;
    vtDecalsPacket.instanceCount = 0;
    vtDecalsPacket.baseVertex = 0;
    vtDecalsPacket.primitiveType = rhi::PRIMITIVE_TRIANGLELIST;
    vtDecalsPacket.vertexLayoutUID = rhi::VertexLayout::UniqueId(vtDecalLayout);
    vtDecalsPacket.startIndex = 0;

    blitPacket.vertexStreamCount = 1;
    blitPacket.vertexCount = 4;
    blitPacket.primitiveType = rhi::PRIMITIVE_TRIANGLELIST;
    blitPacket.primitiveCount = 2;

    rhi::VertexLayout blitQuadLayout;
    blitQuadLayout.AddElement(rhi::VS_POSITION, 0, rhi::VDT_FLOAT, 3);
    blitPacket.vertexLayoutUID = rhi::VertexLayout::UniqueId(blitQuadLayout);

    blendTerrainPacket = blitPacket;
}

void VTDecalPageRenderer::SetVTDecalManager(VTDecalManager* manager)
{
    vtDecalManager = manager;
}

void VTDecalPageRenderer::InitTerrainBlendTargets(const PageRenderParams& params)
{
    blendTargetsTerrain.clear();

    //GFX_COMPLETE - packing data
    blendTargetsTerrain.push_back(RefPtr<Texture>(Texture::CreateFBO(params.pageSize, params.pageSize, FORMAT_RGBA8888)));
    blendTargetsTerrain.push_back(RefPtr<Texture>(Texture::CreateFBO(params.pageSize, params.pageSize, FORMAT_RGBA8888)));
    blendTargetsTerrain.push_back(RefPtr<Texture>(Texture::CreateFBO(params.pageSize, params.pageSize, FORMAT_RGBA8888)));
    blendTargetTerrainSize = params.pageSize;
}

bool VTDecalPageRenderer::RenderPage(const PageRenderParams& params)
{
    if (!vtDecalManager)
        return false;

    vtPageInfo[0] = params.pageBBox.min.x;
    vtPageInfo[1] = params.pageBBox.min.y;
    vtPageInfo[2] = params.pageBBox.max.x - params.pageBBox.min.x;
    vtPageInfo[3] = params.pageBBox.max.y - params.pageBBox.min.y;
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_VT_PAGE_INFO, vtPageInfo, DynamicBindings::UPDATE_SEMANTIC_ALWAYS);

    //clip
    vtDecalManager->Clip(AABBox2(params.pageBBox.min.xy(), params.pageBBox.max.xy()), clipResult);
    //GFX_COMPLETE filter the result by relative size

    uint32 decalCount = static_cast<uint32>(clipResult.size());
    if (decalCount == 0)
        return false;

    uint32 layerCount = uint32(params.pageDst.size());
    for (uint32 i = 0; i < layerCount; i++)
        Renderer::GetDynamicBindings().SetDynamicTexture(static_cast<DynamicBindings::eTextureSemantic>(DynamicBindings::DYNAMIC_TEXTURE_SRC_0 + i), params.pageSrc[i]->handle);

    //GFX_COMPLETE for now we assume decoration is only suppressed by decals - no intermediate blend is required - MUL blending will be ok
    bool useBlendTarget = (decalCount > 1) && (params.component == LandscapePageRenderer::eLandscapeComponent::COMPONENT_TERRAIN);

    //useBlendTarget = false;
    //setup pass
    rhi::RenderPassConfig passConfig;
    passConfig.name = ProfilerGPUMarkerName::LANDSCAPE_PAGE_UPDATE_VT_DECALS;
    passConfig.priority = PRIORITY_SERVICE_3D + 20;

    if (useBlendTarget)
    {
        DVASSERT(params.component == LandscapePageRenderer::eLandscapeComponent::COMPONENT_TERRAIN);
        DVASSERT(layerCount == 2); //in case of intermediate blend we need to have exect layouts
        if (blendTargetsTerrain.empty() || params.pageSize != blendTargetTerrainSize)
            InitTerrainBlendTargets(params);

        DVASSERT(blendTargetsTerrain.size() == 3); //a bit of paranoia

        //setup blend targets
        for (uint32 l = 0; l < 3; ++l)
        {
            passConfig.colorBuffer[l].texture = blendTargetsTerrain[l]->handle;
            passConfig.colorBuffer[l].loadAction = rhi::LOADACTION_CLEAR;
            passConfig.colorBuffer[l].storeAction = rhi::STOREACTION_STORE;
            RhiUtils::SetTargetClearColor(passConfig, 0.0, 0.0, 0.0, 0.0, l);
        }
    }
    else
    {
        for (uint32 l = 0; l < layerCount; ++l)
        {
            passConfig.colorBuffer[l].texture = params.pageDst[l]->handle;
            passConfig.colorBuffer[l].loadAction = rhi::LOADACTION_CLEAR; //GFX_COMPLETE - on-tiler optimizations for framebuffer fetch
            passConfig.colorBuffer[l].storeAction = rhi::STOREACTION_STORE;
        }
    }

    passConfig.depthStencilBuffer.texture = rhi::InvalidHandle;
    passConfig.viewport.x = 0;
    passConfig.viewport.y = 0;
    passConfig.viewport.width = params.pageSize;
    passConfig.viewport.height = params.pageSize;

    DAVA_PROFILER_GPU_RENDER_PASS(passConfig, ProfilerGPUMarkerName::LANDSCAPE_PAGE_UPDATE_VT_DECALS);

    rhi::HRenderPass pass = rhi::AllocateRenderPass(passConfig, 1, &packetList);
    rhi::BeginRenderPass(pass);
    rhi::BeginPacketList(packetList);

    //blit source data (in case of intermediate blend target it should be part of result blend pass)
    if (!useBlendTarget)
        BlitSource(params);
    const FastName& pathName = (params.component == LandscapePageRenderer::eLandscapeComponent::COMPONENT_TERRAIN) ? (useFetch ? PASS_VT_FETCH : (useBlendTarget ? PASS_VT_BLEND : PASS_VT)) : PASS_VT_DECORATION;
    //draw to dest
    std::stable_sort(clipResult.begin(), clipResult.end(), [](const DecalRenderObject* lhs, const DecalRenderObject* rhs) { return lhs->GetSortingKey() < rhs->GetSortingKey(); });
    for (uint32 i = 0; i < decalCount; ++i)
    {
        NMaterial* material = clipResult[i]->GetMaterial();
        if (material && material->PreBuildMaterial(pathName))
        {
            const Matrix4* decalTransform = clipResult[i]->GetWorldTransformPtr();
            vtPos[0] = decalTransform->data[12];
            vtPos[1] = decalTransform->data[13];
            vtBasis[0] = decalTransform->data[0];
            vtBasis[1] = decalTransform->data[1];
            vtBasis[2] = decalTransform->data[4];
            vtBasis[3] = decalTransform->data[5];
            Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_VT_POS, vtPos, DynamicBindings::UPDATE_SEMANTIC_ALWAYS);
            Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_VT_BASIS, vtBasis, DynamicBindings::UPDATE_SEMANTIC_ALWAYS);

            material->BindParams(vtDecalsPacket);

            if (clipResult[i]->GetSplineData() == nullptr)
                RenderDecal(clipResult[i]);
            else
                RenderSpline(clipResult[i]);
        }
    }

    rhi::EndPacketList(packetList);
    rhi::EndRenderPass(pass);

    if (useBlendTarget)
    {
        BlitSourceWithTerrainTargets(params);
    }

    clipResult.clear();
    return true;
}

void VTDecalPageRenderer::RenderDecal(DecalRenderObject* decal)
{
    //no baking for now - mostly probable it will have not much sense as decals are rendered per-page anyway
    DynamicBufferAllocator::AllocResultVB target = DynamicBufferAllocator::AllocateVertexBuffer(sizeof(VTDecalVertex), 4);
    VTDecalVertex* vertices = reinterpret_cast<VTDecalVertex*>(target.data);
    vtDecalsPacket.vertexCount = 6;
    vtDecalsPacket.primitiveCount = 2;
    vtDecalsPacket.indexBuffer = DynamicBufferAllocator::AllocateQuadListIndexBuffer(1);
    vtDecalsPacket.vertexStream[0] = target.buffer;
    vtDecalsPacket.baseVertex = target.baseVertex;
    /*Matrix4* decalTransform = decal->GetWorldTransformPtr();
    //basis and offset are passed through vertex data for further baking possibilities
    Vector2 offset = decalTransform->GetTranslationVector().xy();
    Vector4 basis = Vector4(decalTransform->data[0], decalTransform->data[1], decalTransform->data[4], decalTransform->data[5]);*/
    Vector4 tangents = Vector4(1.0f, 0.0f, 0.0f, 1.0f); //trivial for regular decals - used for splines

    Vector3 decalSize = decal->GetDecalSize();

    vertices[0].position = Vector2(-decalSize.x, -decalSize.y);
    vertices[0].uv = Vector3(0.0f, 1.0f, 1.0f);
    vertices[0].tangents = tangents;

    vertices[1].position = Vector2(decalSize.x, -decalSize.y);
    vertices[1].uv = Vector3(1.0f, 1.0f, 1.0f);
    vertices[1].tangents = tangents;

    vertices[2].position = Vector2(-decalSize.x, decalSize.y);
    vertices[2].uv = Vector3(0.0f, 0.0f, 1.0f);
    vertices[2].tangents = tangents;

    vertices[3].position = Vector2(decalSize.x, decalSize.y);
    vertices[3].uv = Vector3(1.0f, 0.0f, 1.0f);
    vertices[3].tangents = tangents;

    rhi::AddPacket(packetList, vtDecalsPacket);
}

void VTDecalPageRenderer::RenderSpline(DecalRenderObject* decal)
{
    /*packet.vertexStreamCount = instanceCount ? 2 : 1;
	packet.vertexStream[0] = vertexBuffer;
	packet.vertexStream[1] = instanceCount ? instanceBuffer : rhi::HVertexBuffer();
	packet.instanceCount = instanceCount;
	packet.baseVertex = vertexBase;
	packet.vertexCount = vertexCount;
	packet.indexBuffer = indexBuffer;
	packet.primitiveType = primitiveType;
	packet.primitiveCount = CalculatePrimitiveCount(indexCount, primitiveType);
	packet.vertexLayoutUID = vertexLayoutId;
	packet.startIndex = startIndex;*/
    DecalRenderObject::SplineRenderData* splineData = decal->GetSplineData();
    vtDecalsPacket.vertexCount = splineData->sliceCount * 3;
    vtDecalsPacket.primitiveCount = (splineData->sliceCount - 1) * 4;
    vtDecalsPacket.indexBuffer = splineData->indexBuffer;
    vtDecalsPacket.vertexStream[0] = splineData->vertexBuffer;
    vtDecalsPacket.baseVertex = 0;
    if (decal->GetWireframe())
        vtDecalsPacket.options |= rhi::Packet::OPT_WIREFRAME;
    rhi::AddPacket(packetList, vtDecalsPacket);
}

void VTDecalPageRenderer::BlitSource(const PageRenderParams& params)
{
    uint32 layerCount = uint32(params.pageDst.size());
    if (nullptr == blitMaterial)
    {
        blitMaterial = new NMaterial();
        blitMaterial->SetFXName(NMaterialName::TEXTURE_BLIT);
        if (Renderer::GetAPI() == rhi::RHI_METAL)
            blitMaterial->AddFlag(FastName("FLIP_Y"), 1);
        blitMaterial->AddFlag(NMaterialFlagName::FLAG_TEXTURE_COUNT, static_cast<int32>(params.pageSrc.size()));
    }

    if (!blitMaterial->PreBuildMaterial(PASS_FORWARD))
        return;

    blitMaterial->BindParams(blitPacket);

    DynamicBufferAllocator::AllocResultVB vb = DynamicBufferAllocator::AllocateVertexBuffer(sizeof(Vector3), 4);
    blitPacket.vertexStream[0] = vb.buffer;
    blitPacket.baseVertex = vb.baseVertex;
    blitPacket.vertexCount = vb.allocatedVertices;
    blitPacket.indexBuffer = DynamicBufferAllocator::AllocateQuadListIndexBuffer(1);

    Vector3* blitVBData = reinterpret_cast<Vector3*>(vb.data);
    blitVBData[0] = Vector3(-1.f, -1.f, .0f);
    blitVBData[1] = Vector3(-1.f, 1.f, .0f);
    blitVBData[2] = Vector3(1.f, -1.f, .0f);
    blitVBData[3] = Vector3(1.f, 1.f, .0f);

    rhi::AddPacket(packetList, blitPacket);
}

void VTDecalPageRenderer::BlitSourceWithTerrainTargets(const PageRenderParams& params)
{
    DVASSERT(blendTargetsTerrain.size() == 3);
    if (nullptr == blendTerrainMaterial)
    {
        blendTerrainMaterial = new NMaterial();
        blendTerrainMaterial->SetFXName(NMaterialName::VT_COMBINE_BLEND);
        if (Renderer::GetAPI() == rhi::RHI_METAL)
            blendTerrainMaterial->AddFlag(FastName("FLIP_Y"), 1);
        blendTerrainMaterial->AddTexture(FastName("blendedAlbedo"), blendTargetsTerrain[0].Get());
        blendTerrainMaterial->AddTexture(FastName("blendedNormal"), blendTargetsTerrain[1].Get());
        blendTerrainMaterial->AddTexture(FastName("blendedHeight"), blendTargetsTerrain[2].Get()); //GFX_COMPLETE switch it of in case we dont use microtesselation
    }

    if (!blendTerrainMaterial->PreBuildMaterial(PASS_VT))
        return;

    blendTerrainMaterial->BindParams(blendTerrainPacket);

    DynamicBufferAllocator::AllocResultVB vb = DynamicBufferAllocator::AllocateVertexBuffer(sizeof(Vector3), 4);
    blendTerrainPacket.vertexStream[0] = vb.buffer;
    blendTerrainPacket.baseVertex = vb.baseVertex;
    blendTerrainPacket.vertexCount = vb.allocatedVertices;
    blendTerrainPacket.indexBuffer = DynamicBufferAllocator::AllocateQuadListIndexBuffer(1);

    Vector3* blitVBData = reinterpret_cast<Vector3*>(vb.data);
    blitVBData[0] = Vector3(-1.f, -1.f, .0f);
    blitVBData[1] = Vector3(-1.f, 1.f, .0f);
    blitVBData[2] = Vector3(1.f, -1.f, .0f);
    blitVBData[3] = Vector3(1.f, 1.f, .0f);

    //setup pass
    uint32 layerCount = uint32(params.pageDst.size());
    rhi::RenderPassConfig passConfig;
    passConfig.name = ProfilerGPUMarkerName::LANDSCAPE_PAGE_UPDATE_VT_DECALS;
    passConfig.priority = PRIORITY_SERVICE_3D + 20; //stable sort
    for (uint32 l = 0; l < layerCount; ++l)
    {
        passConfig.colorBuffer[l].texture = params.pageDst[l]->handle;
        passConfig.colorBuffer[l].loadAction = rhi::LOADACTION_CLEAR;
        passConfig.colorBuffer[l].storeAction = rhi::STOREACTION_STORE;
    }
    passConfig.depthStencilBuffer.texture = rhi::InvalidHandle;
    passConfig.viewport.x = 0;
    passConfig.viewport.y = 0;
    passConfig.viewport.width = params.pageSize;
    passConfig.viewport.height = params.pageSize;
    DAVA_PROFILER_GPU_RENDER_PASS(passConfig, ProfilerGPUMarkerName::LANDSCAPE_PAGE_UPDATE_VT_DECALS);
    rhi::HPacketList packetList;
    rhi::HRenderPass pass = rhi::AllocateRenderPass(passConfig, 1, &packetList);
    rhi::BeginRenderPass(pass);
    rhi::BeginPacketList(packetList);

    rhi::AddPacket(packetList, blendTerrainPacket);

    rhi::EndPacketList(packetList);
    rhi::EndRenderPass(pass);
}
}
