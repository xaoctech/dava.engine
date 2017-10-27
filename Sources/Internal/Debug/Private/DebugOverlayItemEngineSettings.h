#pragma once

#include "Debug/DebugOverlayItem.h"

namespace DAVA
{
    class DebugOverlayItemEngineSettings : public DebugOverlayItem
    {
    public:
        virtual String GetName() override;
        virtual void Draw() override;
    };
}