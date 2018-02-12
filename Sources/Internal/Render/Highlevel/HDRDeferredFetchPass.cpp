#include "DeferredLightsRenderer.h"
#include "Render/ShaderCache.h"
#include "Debug/ProfilerGPU.h"
#include "Debug/ProfilerMarkerNames.h"
#include "Render/3D/MeshUtils.h"
#include "Render/Highlevel/Light.h"
#include "Render/DynamicBufferAllocator.h"

//for debug dump gbuffers
#include "Logger/Logger.h"
#include "Render/Image/Image.h"

namespace DAVA
{
DeferredLightsRenderer::DeferredLightsRenderer(bool useFetch_)
{
    useFetch = useFetch_;
    unityCube = MeshUtils::BuildAABox(Vector3(1.0f, 1.0f, 1.0f));

    rhi::DepthStencilState::Descriptor dsDescr;
    dsDescr.depthTestEnabled = 0;
    dsDescr.depthFunc = rhi::CMP_LESS;
    dsDescr.depthWriteEnabled = 0;

    rhi::VertexLayout deferredLightLayout;
    deferredLightLayout.AddStream(rhi::VDF_PER_VERTEX);
    deferredLightLayout.AddElement(rhi::VS_POSITION, 0, rhi::VDT_FLOAT, 3); //vertex position
    deferredLightLayout.AddStream(rhi::VDF_PER_INSTANCE);
    deferredLightLayout.AddElement(rhi::VS_TEXCOORD, 0, rhi::VDT_FLOAT, 4); //(lightPosition.xyz, radius)
    deferredLightLayout.AddElement(rhi::VS_TEXCOORD, 1, rhi::VDT_FLOAT, 4); //(color.rgb, shadowIndex)

    deferredLightsPacket.vertexStreamCount = 2;
    deferredLightsPacket.vertexStream[0] = unityCube->vertexBuffer;
    deferredLightsPacket.instanceCount = 0;
    deferredLightsPacket.baseVertex = 0;
    deferredLightsPacket.vertexCount = unityCube->vertexCount;
    deferredLightsPacket.indexBuffer = unityCube->indexBuffer;
    deferredLightsPacket.primitiveType = unityCube->primitiveType;
    deferredLightsPacket.primitiveCount = CalculatePrimitiveCount(unityCube->indexCount, unityCube->primitiveType);
    deferredLightsPacket.vertexLayoutUID = rhi::VertexLayout::UniqueId(deferredLightLayout);
    deferredLightsPacket.startIndex = 0;
    deferredLightsPacket.depthStencilState = rhi::AcquireDepthStencilState(dsDescr);
    //deferredLightsPacket.cullMode = rhi::CULL_CW;
    InvalidateMaterials();
}

DeferredLightsRenderer::~DeferredLightsRenderer()
{
    SafeRelease(unityCube);
}

void DeferredLightsRenderer::InvalidateMaterials()
{
    UnorderedMap<FastName, int32> flags;
    if (useFetch)
        flags[NMaterialFlagName::FLAG_USE_FRAMEBUFFER_FETCH] = 1;

    deferredLightsShader = ShaderDescriptorCache::GetShaderDescriptor(FastName("~res:/Materials2/Shaders/deferred-light"), flags);
    if (!deferredLightsShader->IsValid())
        return;

    deferredLightsPacket.renderPipelineState = deferredLightsShader->GetPiplineState();
    deferredLightsPacket.vertexConstCount = static_cast<uint32>(deferredLightsShader->GetVertexConstBuffersCount());
    deferredLightsPacket.fragmentConstCount = static_cast<uint32>(deferredLightsShader->GetFragmentConstBuffersCount());
    for (const ConstBufferDescriptor& bufferDescriptor : deferredLightsShader->GetConstBufferDescriptors())
    {
        // const rhi::ShaderPropList& props = ShaderDescriptor::GetProps(bufferDescriptor.propertyLayoutId);
        // only auto bindings - all material bindings should be passed via lightInstanceBuffers

        DVASSERT(bufferDescriptor.updateType == rhi::ShaderProp::SOURCE_AUTO);

        rhi::HConstBuffer buffer = deferredLightsShader->GetDynamicBuffer(bufferDescriptor.type, bufferDescriptor.targetSlot);

        if (bufferDescriptor.type == ConstBufferDescriptor::Type::Vertex)
            deferredLightsPacket.vertexConst[bufferDescriptor.targetSlot] = buffer;
        else
            deferredLightsPacket.fragmentConst[bufferDescriptor.targetSlot] = buffer;
    }
}

void DeferredLightsRenderer::Draw(RenderHierarchy::ClipResult& visibilityArray, rhi::HPacketList packetList)
{
    const Vector<Light*>& dynamicLights = visibilityArray.lightArray;
    uint32 lightsCount = uint32(dynamicLights.size());
    if (deferredLightsShader->IsValid() && lightsCount)
    {
        deferredLightsShader->UpdateDynamicParams();
        DAVA_PROFILER_GPU_PACKET(deferredLightsPacket, ProfilerGPUMarkerName::DEFERRED_LIGHTS);

        struct LightInstanceData
        {
            Vector3 pos;
            float radius;
            float color[3];
            float shadowId;
        };

        uint32 processedLights = 0;

        if (initTextureSet && !useFetch)
        {
            rhi::TextureSetDescriptor textureDescr;
            rhi::SamplerState::Descriptor samplerDescr;
            textureDescr.fragmentTextureCount = 4;
            samplerDescr.fragmentSamplerCount = 4;

            textureDescr.fragmentTexture[0] = Renderer::GetRuntimeTextures().GetRuntimeTexture(RuntimeTextures::TEXTURE_GBUFFER_0);
            textureDescr.fragmentTexture[1] = Renderer::GetRuntimeTextures().GetRuntimeTexture(RuntimeTextures::TEXTURE_GBUFFER_1);
            textureDescr.fragmentTexture[2] = Renderer::GetRuntimeTextures().GetRuntimeTexture(RuntimeTextures::TEXTURE_GBUFFER_2);
            textureDescr.fragmentTexture[3] = Renderer::GetRuntimeTextures().GetRuntimeTexture(RuntimeTextures::TEXTURE_GBUFFER_3);

            samplerDescr.fragmentSampler[0] = Renderer::GetRuntimeTextures().GetRuntimeTextureSamplerState(RuntimeTextures::TEXTURE_GBUFFER_0);
            samplerDescr.fragmentSampler[1] = Renderer::GetRuntimeTextures().GetRuntimeTextureSamplerState(RuntimeTextures::TEXTURE_GBUFFER_1);
            samplerDescr.fragmentSampler[2] = Renderer::GetRuntimeTextures().GetRuntimeTextureSamplerState(RuntimeTextures::TEXTURE_GBUFFER_2);
            samplerDescr.fragmentSampler[3] = Renderer::GetRuntimeTextures().GetRuntimeTextureSamplerState(RuntimeTextures::TEXTURE_GBUFFER_3);

            deferredLightsPacket.textureSet = rhi::AcquireTextureSet(textureDescr);
            deferredLightsPacket.samplerState = rhi::AcquireSamplerState(samplerDescr);

            initTextureSet = false;
        }

        do
        {
            DynamicBufferAllocator::AllocResultInstnceBuffer target = DynamicBufferAllocator::AllocateInstanceBuffer(8 * sizeof(float32), lightsCount - processedLights);
            deferredLightsPacket.vertexStream[1] = target.buffer;
            deferredLightsPacket.instanceCount = target.allocatedInstances;
            LightInstanceData* instanceDataPtr = reinterpret_cast<LightInstanceData*>(target.data);
            for (uint32 i = 0; i < target.allocatedInstances; ++i)
            {
                instanceDataPtr[i].pos = dynamicLights[processedLights + i]->GetPosition();
                instanceDataPtr[i].radius = dynamicLights[processedLights + i]->GetRadius();
                const Color& color = dynamicLights[processedLights + i]->GetColor();
                instanceDataPtr[i].color[0] = color.r;
                instanceDataPtr[i].color[1] = color.g;
                instanceDataPtr[i].color[2] = color.b;
                instanceDataPtr[i].shadowId = -1.0f; //GFX_COMPLETE shadow id here later
            }
            //deferredLightsPacket.options = rhi::Packet::OPT_WIREFRAME;
            rhi::AddPacket(packetList, deferredLightsPacket);
            processedLights += target.allocatedInstances;
        } while (processedLights < lightsCount);
    }
}
}
