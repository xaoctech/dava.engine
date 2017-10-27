#pragma once

#include "Debug/DebugOverlayItem.h"

namespace DAVA
{
    class DebugOverlayItemLogger : public DebugOverlayItem
    {
    public:
        DebugOverlayItemLogger();
        ~DebugOverlayItemLogger();
        
       virtual String GetName() override;
        virtual void Draw() override;
    };
}