#include "HDRForwardPass.h"
#include "Render/Highlevel/PostEffectRenderer.h"
#include "Render/Highlevel/ReflectionRenderer.h"
#include "Debug/ProfilerGPU.h"
#include "Debug/ProfilerMarkerNames.h"
#include "Scene3D/Systems/QualitySettingsSystem.h"

namespace DAVA
{
HDRForwardPass::HDRForwardPass(bool cmb)
    : ForwardPass(cmb ? PASS_FORWARD_COMBINE : PASS_FORWARD)
    , inplaceCombine(cmb)
{
}

void HDRForwardPass::Draw(RenderSystem* renderSystem, uint32 drawLayersMask)
{
    DAVA_PROFILER_GPU_RENDER_PASS(passConfig, ProfilerGPUMarkerName::RENDER_PASS_MAIN_3D);

    if (inplaceCombine && (!hdrTexture.IsValid() || !luminanceTexture.IsValid()))
    {
        Size2i sizei = Renderer::GetRuntimeTextures().GetRuntimeTextureSize(RuntimeTextures::TEXTURE_SHARED_DEPTHBUFFER);
        rhi::Texture::Descriptor textureConfig;
        textureConfig.needRestore = false;
        textureConfig.isRenderTarget = true;
        textureConfig.cpuAccessRead = false;
        textureConfig.cpuAccessWrite = false;
        textureConfig.width = uint32(sizei.dx);
        textureConfig.height = uint32(sizei.dy);

        textureConfig.memoryless = true;
        textureConfig.format = rhi::TEXTURE_FORMAT_RGBA16F;
        hdrTexture = rhi::CreateTexture(textureConfig);

        textureConfig.memoryless = false;
        textureConfig.format = rhi::TEXTURE_FORMAT_R16F;
        luminanceTexture = rhi::CreateTexture(textureConfig);
    }

    rhi::RenderPassConfig originalPassConfig = passConfig;

    PostEffectRenderer* postEffectRenderer = renderSystem->GetPostEffectRenderer();
    Camera* mainCamera = renderSystem->GetMainCamera();
    Camera* drawCamera = renderSystem->GetDrawCamera();

    if (inplaceCombine)
    {
        passConfig.colorBuffer[0].storeAction = rhi::STOREACTION_STORE;
        passConfig.colorBuffer[1].texture = hdrTexture;
        passConfig.colorBuffer[1].storeAction = rhi::STOREACTION_NONE;
        passConfig.colorBuffer[2].texture = luminanceTexture;
        passConfig.colorBuffer[2].storeAction = rhi::STOREACTION_STORE;
        passConfig.explicitColorBuffersCount = 3;
        postEffectRenderer->SetFrameContext(hdrTexture, rhi::HTexture(passConfig.colorBuffer[0].texture), passConfig.viewport);
    }
    else
    {
        rhi::AntialiasingType aaType = QualitySettingsSystem::Instance()->GetCurrentQualityValue<QualityGroup::Antialiasing>();
        passConfig.colorBuffer[0].texture = postEffectRenderer->GetHDRTarget();
        passConfig.colorBuffer[0].storeAction = (passConfig.UsingMSAA()) ? rhi::STOREACTION_RESOLVE : rhi::STOREACTION_STORE;
        passConfig.depthStencilBuffer.texture = Renderer::GetRuntimeTextures().GetRuntimeTexture(RuntimeTextures::TEXTURE_SHARED_DEPTHBUFFER);
        passConfig.antialiasingType = (rhi::DeviceCaps().SupportsAntialiasingType(aaType)) ? aaType : rhi::AntialiasingType::NONE;
        passConfig.viewport = rhi::Viewport(0, 0, passConfig.viewport.width, passConfig.viewport.height);
        passConfig.priority = PRIORITY_MAIN_3D + 5;
        SetRenderTargetProperties(passConfig.viewport.width, passConfig.viewport.height, PixelFormat::FORMAT_RGBA16F);
    }

    // Bind camera and render main pass
    SetupCameraParams(mainCamera, drawCamera);

    if (BeginRenderPass(passConfig))
    {
        PrepareVisibilityArrays(mainCamera, renderSystem);
        DrawLayers(mainCamera, drawLayersMask);
        if (inplaceCombine)
        {
            postEffectRenderer->Combine(PostEffectRenderer::CombineMode::InplaceForward, packetList);
        }
        EndRenderPass();
    }

    if (inplaceCombine)
    {
        Size2i luminanceTextureSize = Size2i(passConfig.viewport.width, passConfig.viewport.height);
        postEffectRenderer->DownsampleLuminanceInplace(luminanceTexture, luminanceTextureSize, luminanceTextureSize, -3);
    }

    passConfig = originalPassConfig;

    if (inplaceCombine)
    {
        postEffectRenderer->Debug();
    }
    else
    {
        postEffectRenderer->Render(passConfig.colorBuffer[0].texture, passConfig.viewport);
    }
}

HDRForwardPass::~HDRForwardPass()
{
    rhi::DeleteTexture(hdrTexture);
    rhi::DeleteTexture(luminanceTexture);
}
}
