#pragma once

#include "Debug/DebugOverlayItem.h"

namespace DAVA
{
class DebugOverlayItemRenderOptions final : public DebugOverlayItem
{
public:
    DebugOverlayItemRenderOptions() = default;
    ~DebugOverlayItemRenderOptions() = default;

    virtual String GetName() const override;
    virtual void Draw() override;
};
}