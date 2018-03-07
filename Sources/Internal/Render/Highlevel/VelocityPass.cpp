#include "Render/Highlevel/VelocityPass.h"

#include "Debug/ProfilerGPU.h"
#include "Debug/ProfilerMarkerNames.h"
#include "Render/Highlevel/RenderLayer.h"
#include "Render/ShaderCache.h"
#include "Render/RhiUtils.h"

namespace DAVA
{
VelocityPass::VelocityPass()
    : RenderPass(PASS_VELOCITY)
{
    passConfig.name = "VelocityPass";

    AddRenderLayer(new RenderLayer(RENDER_LAYER_OPAQUE_ID, RenderLayer::LAYER_SORTING_FLAGS_OPAQUE));
    passConfig.usesReverseDepth = rhi::DeviceCaps().isReverseDepthSupported;

    Size2i textureSize = Renderer::GetRuntimeTextures().GetRuntimeTextureSize(RuntimeTextures::TEXTURE_VELOCITY);

    rt = Texture::CreateFBO(textureSize.dx, textureSize.dy, PixelFormat::FORMAT_RG16F);
    passConfig.colorBuffer[0].texture = Renderer::GetRuntimeTextures().GetRuntimeTexture(RuntimeTextures::TEXTURE_VELOCITY);
    passConfig.colorBuffer[0].loadAction = rhi::LOADACTION_CLEAR;
    passConfig.colorBuffer[0].storeAction = rhi::STOREACTION_STORE;
    passConfig.colorBuffer[0].clearColor[0] = 0.0f;
    passConfig.colorBuffer[0].clearColor[1] = 0.0f;
    passConfig.colorBuffer[0].clearColor[2] = 0.0f;
    passConfig.colorBuffer[0].clearColor[3] = 0.0f;

    passConfig.depthStencilBuffer.texture = Renderer::GetRuntimeTextures().GetRuntimeTexture(RuntimeTextures::TEXTURE_SHARED_DEPTHBUFFER);
    passConfig.depthStencilBuffer.loadAction = rhi::LOADACTION_LOAD;
    passConfig.depthStencilBuffer.storeAction = rhi::STOREACTION_NONE;

    SetRenderTargetProperties(textureSize.dx, textureSize.dy, PixelFormat::FORMAT_RG16F);

    velocityMaterial = new NMaterial();
    velocityMaterial->SetFXName(NMaterialName::VELOCITY);

    static const Array<Vector3, 6> quad =
    {
      Vector3(-1.f, -1.f, 1.f), Vector3(-1.f, 1.f, 1.f), Vector3(1.f, -1.f, 1.f), Vector3(1.f, 1.f, 1.f)
    };

    static const Array<uint32, 6> ind = { 0, 1, 2, 1, 3, 2 };

    rhi::VertexBuffer::Descriptor vDesc;
    vDesc.size = sizeof(Vector3) * 6;
    vDesc.initialData = quad.data();
    vDesc.usage = rhi::USAGE_STATICDRAW;
    quadBuffer = rhi::CreateVertexBuffer(vDesc);

    rhi::IndexBuffer::Descriptor iDesc;
    iDesc.size = uint32(6 * sizeof(uint32));
    iDesc.indexSize = rhi::INDEX_SIZE_32BIT;
    iDesc.initialData = ind.data();
    iDesc.usage = rhi::USAGE_STATICDRAW;
    iBuffer = rhi::CreateIndexBuffer(iDesc);

    reprojectVelocityPacket.vertexStreamCount = 1;
    reprojectVelocityPacket.vertexStream[0] = quadBuffer;
    reprojectVelocityPacket.indexBuffer = iBuffer;
    reprojectVelocityPacket.vertexCount = 6;
    reprojectVelocityPacket.primitiveType = rhi::PRIMITIVE_TRIANGLELIST;
    reprojectVelocityPacket.primitiveCount = 2;
    reprojectVelocityPacket.baseVertex = 0;

    rhi::VertexLayout velLayout;
    velLayout.AddElement(rhi::VS_POSITION, 0, rhi::VDT_FLOAT, 3);
    reprojectVelocityPacket.vertexLayoutUID = rhi::VertexLayout::UniqueId(velLayout);

    rhi::DepthStencilState::Descriptor ds;
    ds.depthTestEnabled = false;
    ds.depthWriteEnabled = false;
    ds.depthFunc = rhi::CMP_ALWAYS;
    depthStencilState = rhi::AcquireDepthStencilState(ds);

    GetPrimaryWindow()->draw.Connect(this, &VelocityPass::DebugDraw2D);
}

VelocityPass::~VelocityPass()
{
    SafeRelease(velocityMaterial);
    SafeRelease(rt);
}

void VelocityPass::Draw(RenderSystem* renderSystem, uint32 drawLayersMask /*= 0xFFFFFFFF*/)
{
    DAVA_PROFILER_GPU_RENDER_PASS(passConfig, ProfilerGPUMarkerName::VELOCITY_PASS);

    Camera* mainCamera = renderSystem->GetMainCamera();

    uint32 currVisibilityCriteria = RenderObject::CLIPPING_VISIBILITY_CRITERIA;
    if (!Renderer::GetOptions()->IsOptionEnabled(RenderOptions::ENABLE_STATIC_OCCLUSION))
        currVisibilityCriteria &= ~RenderObject::VISIBLE_STATIC_OCCLUSION;

    visibilityArray.Clear();
    renderSystem->GetRenderHierarchy()->Clip(mainCamera, visibilityArray, currVisibilityCriteria);
    PrepareRenderObjectsToRender(visibilityArray.velocityObjects, mainCamera);

    DrawVisibilityArray(renderSystem, visibilityArray, drawLayersMask);
}

void VelocityPass::DrawVisibilityArray(RenderSystem* renderSystem, RenderHierarchy::ClipResult& preparedVisibilityArray, uint32 drawLayersMask /*= 0xFFFFFFFF*/)
{
    Camera* mainCamera = renderSystem->GetMainCamera();

    ShaderDescriptorCache::ClearDynamicBindigs();
    SetDynamicParams(renderSystem);

    if (!velocityMaterial->PreBuildMaterial(PASS_VELOCITY) || !BeginRenderPass(passConfig))
        return;

    velocityMaterial->BindParams(reprojectVelocityPacket);
    reprojectVelocityPacket.textureSet = RhiUtils::FragmentTextureSet{ Renderer::GetRuntimeTextures().GetRuntimeTexture(RuntimeTextures::TEXTURE_GBUFFER_3) }; // Bind depth.

    rhi::AddPacket(packetList, reprojectVelocityPacket); // Draw the full screen quad for reprojection first.

    ClearLayersArrays();
    if (preparedVisibilityArray.velocityObjects.size() > 0) // Draw all moving objects for this frame above quad.
    {
        PrepareLayersArrays(preparedVisibilityArray.velocityObjects, mainCamera);
        size_t size = renderLayers.size();
        for (size_t k = 0; k < size; ++k)
        {
            RenderLayer* layer = renderLayers[k];

            eRenderLayerID id = layer->GetRenderLayerID();
            if (drawLayersMask & (1 << id))
            {
                RenderBatchArray& batchArray = layersBatchArrays[id];
                batchArray.Sort(mainCamera);
                layer->Draw(mainCamera, batchArray, packetList);
            }
        }
    }

    EndRenderPass();

    bool invertProjection = rhi::IsInvertedProjectionRequired(passConfig.IsRenderTargetPass(), passConfig.IsCubeRenderTargetPass());
    previousVP = renderSystem->GetMainCamera()->GetViewProjMatrix(invertProjection, passConfig.usesReverseDepth);
}

void VelocityPass::DebugDraw2D(Window*)
{
    //if (debugDraw && rt != nullptr)
    //    RenderSystem2D::Instance()->DrawTexture(rt, RenderSystem2D::DEFAULT_2D_TEXTURE_NOBLEND_MATERIAL, Color::White, Rect(50.0f, 270.0f, 256.f, 256.f));
}

void VelocityPass::InvalidateMaterials()
{
    velocityMaterial->InvalidateRenderVariants();
}

void VelocityPass::SetDynamicParams(const RenderSystem* renderSystem)
{
    Vector2 currentOffset = enableFrameJittering ? GetCurrentFrameJitterOffset() : Vector2::Zero;

    prevCurrJitter.x = prevCurrJitter.z;
    prevCurrJitter.y = prevCurrJitter.w;
    prevCurrJitter.z = currentOffset.x;
    prevCurrJitter.w = currentOffset.y;
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_CAMERA_PROJ_JITTER_PREV_CURR, &prevCurrJitter, reinterpret_cast<pointer_size>(&prevCurrJitter));

    viewportSize = Vector2(viewport.dx, viewport.dy);
    rcpViewportSize = Vector2(1.0f / viewport.dx, 1.0f / viewport.dy);
    viewportOffset = Vector2(viewport.x, viewport.y);
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_VIEWPORT_SIZE, &viewportSize, reinterpret_cast<pointer_size>(&viewportSize));
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_RCP_VIEWPORT_SIZE, &rcpViewportSize, reinterpret_cast<pointer_size>(&rcpViewportSize));
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_VIEWPORT_OFFSET, &viewportOffset, reinterpret_cast<pointer_size>(&viewportOffset));

    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_PREV_VIEW_PROJ, &previousVP, reinterpret_cast<pointer_size>(&previousVP));

    Size2i texSize = Renderer::GetRuntimeTextures().GetRuntimeTextureSize(RuntimeTextures::TEXTURE_VELOCITY);
    Vector2 vSize(float32(texSize.dx), float32(texSize.dy));

    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_RENDER_TARGET_SIZE, &vSize, reinterpret_cast<pointer_size>(&vSize));

    Camera* mainCamera = renderSystem->GetMainCamera();
    Camera* drawCamera = renderSystem->GetDrawCamera();
    SetupCameraParams(mainCamera, drawCamera);
}
}