#pragma once

#include "Render/Highlevel/RenderPass.h"
#include "Render/Highlevel/RenderLayer.h"

namespace DAVA
{
class ForwardPass : public RenderPass
{
public:
    ForwardPass(const FastName& name)
        : RenderPass(name)
    {
        AddRenderLayer(new RenderLayer(RENDER_LAYER_OPAQUE_ID, RenderLayer::LAYER_SORTING_FLAGS_OPAQUE));
        AddRenderLayer(new RenderLayer(RENDER_LAYER_AFTER_OPAQUE_ID, RenderLayer::LAYER_SORTING_FLAGS_AFTER_OPAQUE));
        AddRenderLayer(new RenderLayer(RENDER_LAYER_VEGETATION_ID, RenderLayer::LAYER_SORTING_FLAGS_VEGETATION));
        AddRenderLayer(new RenderLayer(RENDER_LAYER_ALPHA_TEST_LAYER_ID, RenderLayer::LAYER_SORTING_FLAGS_ALPHA_TEST_LAYER));
        AddRenderLayer(new RenderLayer(RENDER_LAYER_WATER_ID, RenderLayer::LAYER_SORTING_FLAGS_WATER));
        AddRenderLayer(new RenderLayer(RENDER_LAYER_SKY_ID, 0)); // after all depth writing materials
        AddRenderLayer(new RenderLayer(RENDER_LAYER_TRANSLUCENT_ID, RenderLayer::LAYER_SORTING_FLAGS_TRANSLUCENT));
        AddRenderLayer(new RenderLayer(RENDER_LAYER_AFTER_TRANSLUCENT_ID, RenderLayer::LAYER_SORTING_FLAGS_AFTER_TRANSLUCENT));
        AddRenderLayer(new RenderLayer(RENDER_LAYER_DEBUG_DRAW_ID, RenderLayer::LAYER_SORTING_FLAGS_DEBUG_DRAW));

        passConfig.priority = PRIORITY_MAIN_3D;
        passConfig.usesReverseDepth = rhi::DeviceCaps().isReverseDepthSupported;
    }

    ~ForwardPass() = default;
};
}
