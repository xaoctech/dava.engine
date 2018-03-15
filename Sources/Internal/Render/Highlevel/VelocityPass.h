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
    void DrawVisibilityArray(RenderSystem* renderSystem, RenderHierarchy::ClipResult& preparedVisibilityArray, uint32 drawLayersMask = 0xFFFFFFFF) override;
    void DebugDraw2D(Window*);

    void InvalidateMaterials() override;

private:
    void SetDynamicParams(const RenderSystem* renderSystem);
    Matrix4 previousVP;
    Vector4 prevCurrJitter;

    NMaterial* velocityMaterial = nullptr;
    Texture* rt = nullptr;

    rhi::Packet reprojectVelocityPacket;
    rhi::HVertexBuffer quadBuffer;
    rhi::HIndexBuffer iBuffer;
    rhi::HDepthStencilState depthStencilState;
};
}