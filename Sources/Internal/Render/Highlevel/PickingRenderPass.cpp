#include "Render/Highlevel/PickingRenderPass.h"
#include "Render/Highlevel/RenderPassNames.h"
#include "Render/Highlevel/RenderLayer.h"

#include "Render/Texture.h"
#include "Render/RuntimeTextures.h"

namespace DAVA
{
PickingRenderPass::PickingRenderPass()
    : RenderPass(PASS_PICKING)
{
    AddRenderLayer(new RenderLayer(RENDER_LAYER_OPAQUE_ID, RenderLayer::LAYER_SORTING_FLAGS_OPAQUE));
}

void PickingRenderPass::Draw(RenderSystem* renderSystem, uint32 drawLayersMask /*= 0xFFFFFFFF*/)
{
    rhi::RenderPassConfig originalPassConfig = passConfig;
    passConfig.colorBuffer[0].texture = Renderer::GetRuntimeTextures().GetRuntimeTexture(RuntimeTextures::TEXTURE_UVPICKING);
    passConfig.colorBuffer[0].loadAction = rhi::LOADACTION_CLEAR;
    passConfig.colorBuffer[0].storeAction = rhi::STOREACTION_STORE;
    passConfig.depthStencilBuffer.texture = Renderer::GetRuntimeTextures().GetRuntimeTexture(RuntimeTextures::TEXTURE_SHARED_DEPTHBUFFER);
    passConfig.priority = PRIORITY_SERVICE_2D;
    passConfig.viewport.x = 0;
    passConfig.viewport.y = 0;

    Size2i pickTextureSize = Renderer::GetRuntimeTextures().GetRuntimeTextureSize(RuntimeTextures::TEXTURE_UVPICKING);
    passConfig.viewport.width = pickTextureSize.dx;
    passConfig.viewport.height = pickTextureSize.dy;

    Camera* mainCamera = renderSystem->GetMainCamera();
    Camera* drawCamera = renderSystem->GetDrawCamera();

    SetupCameraParams(mainCamera, drawCamera);

    if (BeginRenderPass(passConfig))
    {
        uint32 currVisibilityCriteria = RenderObject::CLIPPING_VISIBILITY_CRITERIA | RenderObject::VISIBLE_PICKING_PASS;
        if (!Renderer::GetOptions()->IsOptionEnabled(RenderOptions::ENABLE_STATIC_OCCLUSION))
            currVisibilityCriteria &= ~RenderObject::VISIBLE_STATIC_OCCLUSION;

        visibilityArray.Clear();
        renderSystem->GetRenderHierarchy()->Clip(mainCamera, visibilityArray, currVisibilityCriteria);

        ClearLayersArrays();
        PrepareLayersArrays(visibilityArray.geometryArray, mainCamera);

        DrawLayers(mainCamera, drawLayersMask);
        EndRenderPass();
    }

    passConfig = originalPassConfig;
}

} // namespace DAVA