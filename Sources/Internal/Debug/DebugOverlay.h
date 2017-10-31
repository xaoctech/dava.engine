#pragma once

#include "Base/BaseTypes.h"
#include "Base/Vector.h"

namespace DAVA
{
namespace Private
{
class EngineBackend;
}

class Window;
class DebugOverlayItem;
class DebugOverlayItemEngineSettings;
class DebugOverlayItemLogger;

/**
        Class representing visual overlay ment for debugging.

        It provides features for extending and showing custom information or UI via `DebugOverlayItem`.
        Each `DebugOverlayItem` is shown as a menu item which can be enabled or disabled. Multiple items can be enabled simultaneously.
    */
class DebugOverlay final
{
public:
    /** Show overlay. */
    void Show();

    /** Hide overlay. */
    void Hide();

    /** Return `true` if overlay is active, `false` otherwise. */
    bool IsShown() const;

    /** Adds `overlayItem` to the menu. */
    void RegisterItem(DebugOverlayItem* overlayItem);

    /** Removes `overlayItem` from the menu. The item must be registered. */
    void UnregisterItem(DebugOverlayItem* overlayItem);

    /**
            Enables drawing `overlayItem`.
            Calling this method has the same effect as marking the checkbox as enabled in the menu.
            `overlayItem` must be registered.    
        */
    void ShowItem(DebugOverlayItem* overlayItem);

    /**
            Disables drawing of `overlayItem`.
            Calling this method has the same effect as marking the checkbox as disabled in the menu.
            `overlayItem` must be registered.
        */
    void HideItem(DebugOverlayItem* overlayItem);

private:
    DebugOverlay();
    ~DebugOverlay();
    DebugOverlay(const DebugOverlay&) = delete;
    DebugOverlay& operator=(const DebugOverlay&) = delete;

    void OnUpdate(Window* window, float32 timeDelta);

    void RegisterDefaultItems();
    void UnregisterDefaultItems();

private:
    struct ItemData
    {
        DebugOverlayItem* item;
        String name;
        bool shown;
    };

    bool shown = false;
    Vector<ItemData> items;

    // Items added by default
    std::unique_ptr<DebugOverlayItemEngineSettings> defaultItemEngineSettings;
    std::unique_ptr<DebugOverlayItemLogger> defaultItemLogger;

    // For creation
    friend class Private::EngineBackend;
};
}