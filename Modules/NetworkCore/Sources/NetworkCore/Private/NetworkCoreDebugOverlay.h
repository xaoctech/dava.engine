#pragma once

#include <Debug/DebugOverlay.h>
#include <Debug/DebugOverlayItem.h>

namespace DAVA
{
class NetworkCoreDebugOverlayItem final : public DebugOverlayItem
{
public:
    String GetName() const override;
    void Draw() override;
};
}
