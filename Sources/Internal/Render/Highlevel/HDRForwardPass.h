#pragma once

#include "Render/Highlevel/ForwardPass.h"

namespace DAVA
{
class HDRForwardPass : public ForwardPass
{
public:
    HDRForwardPass(bool inplaceCombine);
    ~HDRForwardPass();

    void Draw(RenderSystem* renderSystem, uint32 drawLayersMask = 0xFFFFFFFF) override;

private:
    rhi::HTexture hdrTexture;
    rhi::HTexture luminanceTexture;
    bool inplaceCombine = false;
};
}