#include "UtilityRenderPass.h"
#include "Debug/ProfilerGPU.h"
#include "Debug/ProfilerMarkerNames.h"
#include "Render/RhiUtils.h"

namespace DAVA
{
RescalePass::RescalePass()
    : RenderPass(PASS_RESCALE)
{
    const static uint32 VERTEX_COUNT = 6;
    struct ResVertex
    {
        Vector3 pos;
    };
    std::array<ResVertex, VERTEX_COUNT> quad =
    {
      Vector3(-1.f, -1.f, 1.f), Vector3(-1.f, 1.f, 1.f), Vector3(1.f, -1.f, 1.f),
      Vector3(-1.f, 1.f, 1.f), Vector3(1.f, 1.f, 1.f), Vector3(1.f, -1.f, 1.f)
    };

    rhi::VertexBuffer::Descriptor vDesc;
    vDesc.size = sizeof(ResVertex) * VERTEX_COUNT;
    vDesc.initialData = quad.data();
    vDesc.usage = rhi::USAGE_STATICDRAW;
    quadBuffer = rhi::CreateVertexBuffer(vDesc);
    rhi::VertexLayout vxLayout;
    vxLayout.AddElement(rhi::VS_POSITION, 0, rhi::VDT_FLOAT, 3);

    rescalePacket.vertexLayoutUID = rhi::VertexLayout::UniqueId(vxLayout);

    rescalePacket.vertexStreamCount = 1;
    rescalePacket.vertexStream[0] = quadBuffer;
    rescalePacket.primitiveType = rhi::PRIMITIVE_TRIANGLELIST;
    rescalePacket.primitiveCount = 2;

    rescaleMaterial = new NMaterial();
    rescaleMaterial->SetFXName(NMaterialName::RESCALE_QUAD);
}

void RescalePass::Draw(RenderSystem* renderSystem, uint32 drawLayersMask /* = 0xFFFFFFFF */)
{
    DAVA_PROFILER_GPU_RENDER_PASS(passConfig, ProfilerGPUMarkerName::RESCALE_PASS);
    SetViewport(renderSystem->GetConfiguration().viewport);

    //per pass viewport bindings
    viewportSize = Vector2(renderSystem->GetConfiguration().scaledViewport.dx, renderSystem->GetConfiguration().scaledViewport.dy);
    rcpViewportSize = Vector2(1.0f / renderSystem->GetConfiguration().scaledViewport.dx, 1.0f / renderSystem->GetConfiguration().scaledViewport.dy);
    viewportOffset = Vector2(renderSystem->GetConfiguration().scaledViewport.x, renderSystem->GetConfiguration().scaledViewport.y);

    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_VIEWPORT_SIZE, &viewportSize, reinterpret_cast<pointer_size>(&viewportSize));
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_RCP_VIEWPORT_SIZE, &rcpViewportSize, reinterpret_cast<pointer_size>(&rcpViewportSize));
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_VIEWPORT_OFFSET, &viewportOffset, reinterpret_cast<pointer_size>(&viewportOffset));

    passConfig.priority = renderSystem->GetConfiguration().basePriority - 5;
    passConfig.colorBuffer[0].texture = renderSystem->GetConfiguration().colorTarget;
    passConfig.colorBuffer[0].loadAction = rhi::LOADACTION_CLEAR;
    passConfig.depthStencilBuffer.texture = renderSystem->GetConfiguration().depthTarget;
    //RhiUtils::SetTargetClearColor(passConfig, 1.0f, 1.0f, 0.0, 1.0);
    RhiUtils::SetTargetClearColor(passConfig, renderSystem->GetConfiguration().clearColor);

    if (BeginRenderPass(passConfig))
    {
        SetupCameraParams(renderSystem->GetMainCamera(), renderSystem->GetDrawCamera());
        Size2i rtSize = Renderer::GetRuntimeTextures().GetRuntimeTextureSize(RuntimeTextures::TEXTURE_SHARED_DEPTHBUFFER);
        scaledRtDimensions = Vector2(float32(rtSize.dx), float32(rtSize.dy));
        //GFX_COMPLETE as binding in BeginRenderPass is trash - everything should be set up before
        Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_RENDER_TARGET_SIZE, scaledRtDimensions.data, DynamicBindings::UPDATE_SEMANTIC_ALWAYS);

        //GFX_COMPLETE fix projection flip for all back-ends
        if (rhi::HostApi() == rhi::RHI_METAL)
        {
            bool invertProjection = (passConfig.colorBuffer[0].texture != rhi::InvalidHandle) && (!rhi::DeviceCaps().isUpperLeftRTOrigin);
            float32 cv = invertProjection ? 1.0f : -1.0f;

            if (Renderer::GetCurrentRenderFlow() == RenderFlow::TileBasedHDRDeferred)
                cv = -1.f;

            Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_PROJECTION_FLIPPED, &cv, DynamicBindings::UPDATE_SEMANTIC_ALWAYS);
        }

        if (rescaleMaterial->PreBuildMaterial(PASS_RESCALE))
        {
            rescaleMaterial->BindParams(rescalePacket);
            rhi::AddPacket(packetList, rescalePacket);
        }
        EndRenderPass();
    }
}

void RescalePass::InvalidateMaterials()
{
    rescaleMaterial->InvalidateRenderVariants();
}
}
