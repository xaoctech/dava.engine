#include "DeferredDecalRenderer.h"
#include "Debug/ProfilerGPU.h"
#include "Debug/ProfilerMarkerNames.h"
#include "Render/3D/MeshUtils.h"
#include "Render/DynamicBufferAllocator.h"
#include "Render/Highlevel/DecalRenderObject.h"

#include "Logger/Logger.h"
#include "Render/Image/Image.h"
#include "Render/Highlevel/RenderPassNames.h"

namespace DAVA
{
DeferredDecalRenderer::DeferredDecalRenderer(bool useFetch_)
{
    useFetch = useFetch_;
    //init common
    unityCube = MeshUtils::BuildAABox(Vector3(1.0f, 1.0f, 1.0f));

    //init deferred decals stuff
    rhi::VertexLayout deferredDecalLayout;
    deferredDecalLayout.AddStream(rhi::VDF_PER_VERTEX);
    deferredDecalLayout.AddElement(rhi::VS_POSITION, 0, rhi::VDT_FLOAT, 3); //vertex position
    deferredDecalLayout.AddStream(rhi::VDF_PER_INSTANCE);
    deferredDecalLayout.AddElement(rhi::VS_TEXCOORD, 0, rhi::VDT_FLOAT, 4); //(ex, maskMode);
    deferredDecalLayout.AddElement(rhi::VS_TEXCOORD, 1, rhi::VDT_FLOAT, 4); //(ey, maskId);
    deferredDecalLayout.AddElement(rhi::VS_TEXCOORD, 2, rhi::VDT_FLOAT, 4); //(ez, blendWidth);
    deferredDecalLayout.AddElement(rhi::VS_TEXCOORD, 3, rhi::VDT_FLOAT, 4); //(p, 0);

    deferredDecalsPacket.vertexStreamCount = 2;
    deferredDecalsPacket.vertexStream[0] = unityCube->vertexBuffer;
    deferredDecalsPacket.instanceCount = 0;
    deferredDecalsPacket.baseVertex = 0;
    deferredDecalsPacket.vertexCount = unityCube->vertexCount;
    deferredDecalsPacket.indexBuffer = unityCube->indexBuffer;
    deferredDecalsPacket.primitiveType = unityCube->primitiveType;
    deferredDecalsPacket.primitiveCount = CalculatePrimitiveCount(unityCube->indexCount, unityCube->primitiveType);
    deferredDecalsPacket.vertexLayoutUID = rhi::VertexLayout::UniqueId(deferredDecalLayout);
    deferredDecalsPacket.startIndex = 0;
    //deferredLightsPacket.cullMode = rhi::CULL_CW;
}

DeferredDecalRenderer::~DeferredDecalRenderer()
{
    SafeRelease(unityCube);
}

uint32 DeferredDecalRenderer::Draw(RenderHierarchy::ClipResult& visibilityArray, rhi::HPacketList packetList)
{
    if (useFetch)
        return 0; //GFX_COMPLETE - add fetch shaders

    if (visibilityArray.decalArray.empty())
        return 0;

    std::stable_sort(visibilityArray.decalArray.begin(), visibilityArray.decalArray.end(), [](const DecalRenderObject* lhs, const DecalRenderObject* rhs) { return lhs->GetSortingKey() < rhs->GetSortingKey(); });
    uint32 gbuffersMask = 0;
    NMaterial* currGroupMaterial = visibilityArray.decalArray[0]->GetMaterial();
    uint32 currGroupStart = 0;
    uint32 decalCount = static_cast<uint32>(visibilityArray.decalArray.size());
    for (uint32 i = 0; i < decalCount; ++i)
    {
        gbuffersMask |= visibilityArray.decalArray[i]->GetGbufferMask();
        NMaterial* currMaterial = visibilityArray.decalArray[i]->GetMaterial();
        if (currMaterial != currGroupMaterial) //finish current group
        {
            visibleDecalGroups.emplace_back(currGroupStart, i, currGroupMaterial);
            currGroupStart = i;
            currGroupMaterial = currMaterial;
        }
    }
    visibleDecalGroups.emplace_back(currGroupStart, decalCount, currGroupMaterial);

    for (DecalGroup& group : visibleDecalGroups)
    {
        if (group.material && group.material->PreBuildMaterial(useFetch ? PASS_GBUFFER_FETCH : PASS_GBUFFER))
        {
            //note - we are not binding render object dynamic params here, as for instancing they all should be set into instance buffers
            group.material->BindParams(deferredDecalsPacket);
            uint32 currDecal = group.start;
            do
            {
                DynamicBufferAllocator::AllocResultInstnceBuffer target = DynamicBufferAllocator::AllocateInstanceBuffer(16 * sizeof(float32), group.next - currDecal);
                deferredDecalsPacket.vertexStream[1] = target.buffer;
                deferredDecalsPacket.instanceCount = target.allocatedInstances;
                Matrix4* instanceDataPtr = reinterpret_cast<Matrix4*>(target.data);
                for (uint32 i = 0; i < target.allocatedInstances; ++i)
                {
                    DecalRenderObject* decalRO = visibilityArray.decalArray[currDecal];
                    instanceDataPtr[i] = Matrix4::MakeScale(decalRO->GetDecalSize());
                    instanceDataPtr[i] *= *(decalRO->GetWorldTransformPtr());

                    //GFX_COMPLETE - additional decal params here
                    instanceDataPtr[i].data[3] = 0;
                    instanceDataPtr[i].data[7] = 0;
                    instanceDataPtr[i].data[11] = 0;
                    instanceDataPtr[i].data[15] = 1;

                    currDecal++;
                }
                //deferredDecalsPacket.options = rhi::Packet::OPT_WIREFRAME;
                DAVA_PROFILER_GPU_PACKET(deferredDecalsPacket, ProfilerGPUMarkerName::DEFERRED_DECAL_GROUP);
                rhi::AddPacket(packetList, deferredDecalsPacket);
                currDecal += target.allocatedInstances;
            } while (currDecal < group.next);
        }
    }
    visibleDecalGroups.clear();

    //GFX_COMPLETE GFX_TMP - gbuffer copy mask is not in materials meta yet
    gbuffersMask = (1 << GBUFFER_COPY_0) | (1 << GBUFFER_COPY_1) | (1 << GBUFFER_COPY_2);
    //gbuffersMask = 0;

    return gbuffersMask;
}
}
