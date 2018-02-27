#include "Render/Highlevel/VelocityPass.h"

#include "Debug/ProfilerGPU.h"
#include "Debug/ProfilerMarkerNames.h"
#include "Render/RhiUtils.h"

namespace DAVA
{
VelocityPass::VelocityPass()
    : RenderPass(PASS_VELOCITY)
{
    passConfig.colorBuffer[0].clearColor[0] = 0.0f;
    passConfig.colorBuffer[0].clearColor[1] = 0.0f;
    passConfig.colorBuffer[0].clearColor[2] = 0.0f;
    passConfig.colorBuffer[0].clearColor[3] = 0.0f;
    passConfig.colorBuffer[0].texture = Renderer::GetRuntimeTextures().GetRuntimeTexture(RuntimeTextures::TEXTURE_VELOCITY);
    passConfig.colorBuffer[0].loadAction = rhi::LOADACTION_CLEAR;
    passConfig.colorBuffer[0].storeAction = rhi::STOREACTION_STORE;
    passConfig.depthStencilBuffer.texture = rhi::InvalidHandle;
    passConfig.depthStencilBuffer.loadAction = rhi::LOADACTION_NONE;
    passConfig.depthStencilBuffer.storeAction = rhi::STOREACTION_NONE;

    passConfig.usesReverseDepth = rhi::DeviceCaps().isReverseDepthSupported;
    Size2i velocitySize = Renderer::GetRuntimeTextures().GetRuntimeTextureSize(RuntimeTextures::TEXTURE_VELOCITY);
    SetRenderTargetProperties(velocitySize.dx, velocitySize.dy, Renderer::GetRuntimeTextures().GetRuntimeTextureFormat(RuntimeTextures::TEXTURE_VELOCITY));

    velocityMaterial = new NMaterial();
    velocityMaterial->SetFXName(NMaterialName::VELOCITY);
    velocityMaterial->AddProperty(FastName("jitPrevVP"), Matrix4::IDENTITY.data, rhi::ShaderProp::TYPE_FLOAT4X4);
    velocityMaterial->AddProperty(FastName("prevCurrJitter"), Vector4::Zero.data, rhi::ShaderProp::TYPE_FLOAT4);

    Array<Vector3, 6> quad =
    {
      Vector3(-1.f, -1.f, 1.f), Vector3(-1.f, 1.f, 1.f), Vector3(1.f, -1.f, 1.f),
      Vector3(-1.f, 1.f, 1.f), Vector3(1.f, 1.f, 1.f), Vector3(1.f, -1.f, 1.f)
    };

    rhi::VertexBuffer::Descriptor vDesc;
    vDesc.size = sizeof(Vector3) * 6;
    vDesc.initialData = quad.data();
    vDesc.usage = rhi::USAGE_STATICDRAW;
    quadBuffer = rhi::CreateVertexBuffer(vDesc);

    velocityPacket.vertexStreamCount = 1;
    velocityPacket.vertexStream[0] = quadBuffer;
    velocityPacket.vertexCount = 4;
    velocityPacket.primitiveType = rhi::PRIMITIVE_TRIANGLELIST;
    velocityPacket.primitiveCount = 2;

    rhi::VertexLayout velLayout;
    velLayout.AddElement(rhi::VS_POSITION, 0, rhi::VDT_FLOAT, 3);
    velocityPacket.vertexLayoutUID = rhi::VertexLayout::UniqueId(velLayout);

    rhi::DepthStencilState::Descriptor ds;
    ds.depthTestEnabled = false;
    ds.depthWriteEnabled = false;
    ds.depthFunc = rhi::CMP_ALWAYS;
    depthStencilState = rhi::AcquireDepthStencilState(ds);
}

VelocityPass::~VelocityPass()
{
    SafeRelease(velocityMaterial);
}

void VelocityPass::Draw(RenderSystem* renderSystem, uint32 drawLayersMask /*= 0xFFFFFFFF*/)
{
    DAVA_PROFILER_GPU_RENDER_PASS(passConfig, ProfilerGPUMarkerName::VELOCITY_PASS);

    Camera* mainCamera = renderSystem->GetMainCamera();
    Camera* drawCamera = renderSystem->GetDrawCamera();
    SetupCameraParams(mainCamera, drawCamera);

    viewportSize = Vector2(viewport.dx, viewport.dy);
    rcpViewportSize = Vector2(1.0f / viewport.dx, 1.0f / viewport.dy);
    viewportOffset = Vector2(viewport.x, viewport.y);
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_VIEWPORT_SIZE, &viewportSize, reinterpret_cast<pointer_size>(&viewportSize));
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_RCP_VIEWPORT_SIZE, &rcpViewportSize, reinterpret_cast<pointer_size>(&rcpViewportSize));
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_VIEWPORT_OFFSET, &viewportOffset, reinterpret_cast<pointer_size>(&viewportOffset));
    Size2i texSize = Renderer::GetRuntimeTextures().GetRuntimeTextureSize(RuntimeTextures::TEXTURE_VELOCITY);
    Vector2 vSize(float32(texSize.dx), float32(texSize.dy));

    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_RENDER_TARGET_SIZE, &vSize, reinterpret_cast<pointer_size>(&vSize));

    velocityMaterial->SetPropertyValue(FastName("jitPrevVP"), previousVPUnjit.data);

    Vector2 currentOffset = GetCurrentFrameJitterOffset();

    prevCurrJitter.x = prevCurrJitter.z;
    prevCurrJitter.y = prevCurrJitter.w;
    prevCurrJitter.z = currentOffset.x;
    prevCurrJitter.w = currentOffset.y;
    velocityMaterial->SetPropertyValue(FastName("prevCurrJitter"), prevCurrJitter.data);

    if (velocityMaterial->PreBuildMaterial(PASS_VELOCITY))
    {
        velocityMaterial->BindParams(velocityPacket);
        velocityPacket.textureSet = RhiUtils::FragmentTextureSet{ Renderer::GetRuntimeTextures().GetRuntimeTexture(RuntimeTextures::TEXTURE_GBUFFER_3) };
        rhi::HPacketList packetList;
        rhi::HRenderPass pass = rhi::AllocateRenderPass(passConfig, 1, &packetList);

        rhi::BeginRenderPass(pass);
        rhi::BeginPacketList(packetList);

        rhi::AddPacket(packetList, velocityPacket);

        rhi::EndPacketList(packetList);
        rhi::EndRenderPass(pass);
    }
    bool invertProjection = rhi::IsInvertedProjectionRequired(passConfig.IsRenderTargetPass(), passConfig.IsCubeRenderTargetPass());
    previousVPUnjit = mainCamera->GetViewProjMatrix(invertProjection, passConfig.usesReverseDepth);
}

void VelocityPass::InvalidateMaterials()
{
    velocityMaterial->InvalidateRenderVariants();
}
}