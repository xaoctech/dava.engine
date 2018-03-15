#pragma once

#include <UI/Flow/UIFlowService.h>

class FlowDebugOverlayService : public DAVA::UIFlowService
{
    DAVA_VIRTUAL_REFLECTION(FlowDebugOverlayService, UIFlowService);

public:
    FlowDebugOverlayService();

    void ToggleDebugOverlay();

private:
    bool cursorWasPinned = false;
};