#pragma once

#include "Render/Highlevel/ForwardPass.h"

namespace DAVA
{
class LDRForwardPass : public ForwardPass
{
public:
    LDRForwardPass();
    ~LDRForwardPass();

    void Draw(RenderSystem* renderSystem, uint32 drawLayersMask = 0xFFFFFFFF) override;
};
}