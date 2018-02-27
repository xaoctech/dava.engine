#pragma once

#include "Debug/DebugOverlayItem.h"

namespace DAVA
{
class DebugOverlayItemEngineSettings final : public DebugOverlayItem
{
public:
    String GetName() const override;
    void Draw(float32 elapsedTime) override;
};
}
