#pragma once

#include "Base/BaseTypes.h"

namespace DAVA
{
    class DebugOverlayItem
    {
    public:
        virtual String GetName() = 0;
        virtual void Draw() = 0;

        virtual void OnEnabled() {};
        virtual void OnDisabled() {};
    };
}