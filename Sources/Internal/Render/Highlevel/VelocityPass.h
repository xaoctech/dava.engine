#pragma once

#include "Render/Highlevel/RenderPass.h"

namespace DAVA
{
class VelocityPass : public RenderPass
{
public:
    VelocityPass();
    ~VelocityPass();

    void Draw(RenderSystem* renderSystem, uint32 drawLayersMask = 0xFFFFFFFF) override;
    void InvalidateMaterials() override;

private:
    NMaterial* velocityMaterial;
    QuadRenderer quad;
    Matrix4 previousVPUnjit;

    rhi::Packet velocityPacket;
    rhi::HVertexBuffer quadBuffer;
    rhi::HDepthStencilState depthStencilState;

    Vector4 prevCurrJitter;
};
}