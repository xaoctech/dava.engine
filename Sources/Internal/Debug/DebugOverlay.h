#pragma once

#include "Base/BaseTypes.h"
#include "Base/Vector.h"

namespace DAVA
{
    class Window;

    class DebugOverlayItem;

    class DebugOverlay final
    {
    public:
        DebugOverlay();
        ~DebugOverlay();
        DebugOverlay(const DebugOverlay&) = delete;
        DebugOverlay& operator=(const DebugOverlay&) = delete;

        void Enable();
        void Disable();

        void RegisterItem(DebugOverlayItem* overlayItem);
        void UnregisterItem(DebugOverlayItem* overlayItem);

        void EnableItem(DebugOverlayItem* overlayItem);
        void DisableItem(DebugOverlayItem* overlayItem);

    private:
        void OnUpdate(Window* window, float32 timeDelta);

    private:
        struct ItemData
        {
            DebugOverlayItem* item;
            String name;
            bool enabled;
        };

        Vector<ItemData> items;
    };
}