#pragma once

#include "Render/Highlevel/RenderPass.h"

namespace DAVA
{
class Texture;
class PickingRenderPass : public RenderPass
{
public:
    PickingRenderPass();

    void Draw(RenderSystem* renderSystem, uint32 drawLayersMask = 0xFFFFFFFF) override;
};
} // namespace DAVA