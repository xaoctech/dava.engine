#pragma once

#include "Debug/DebugOverlayItem.h"

namespace DAVA
{
class DebugOverlayItemEngineSettings final : public DebugOverlayItem
{
public:
    virtual String GetName() const override;
    virtual void Draw() override;
};
}