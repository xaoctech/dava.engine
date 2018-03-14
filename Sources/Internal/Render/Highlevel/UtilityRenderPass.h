#pragma once

#include "Base/BaseTypes.h"
#include "Base/FastName.h"
#include "Render/Highlevel/RenderPass.h"
#include "Render/Highlevel/RenderPassNames.h"
#include "Render/Highlevel/RenderBatchArray.h"
#include "Render/Highlevel/QuadRenderer.h"

namespace DAVA
{
class RescalePass : public RenderPass
{
public:
    RescalePass();
    ~RescalePass();
    void Draw(RenderSystem* renderSystem, uint32 drawLayersMask = 0xFFFFFFFF) override;
    void InvalidateMaterials() override;

private:
    rhi::Packet rescalePacket;
    NMaterial* rescaleMaterial = nullptr;
    rhi::HVertexBuffer quadBuffer;
    Vector2 scaledRtDimensions;
};
}