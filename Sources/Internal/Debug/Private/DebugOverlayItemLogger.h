#pragma once

#include "Debug/DebugOverlayItem.h"

namespace DAVA
{
class DebugOverlayItemLogger final : public DebugOverlayItem
{
public:
    DebugOverlayItemLogger();
    ~DebugOverlayItemLogger();

    virtual String GetName() const override;
    virtual void Draw() override;
};
}