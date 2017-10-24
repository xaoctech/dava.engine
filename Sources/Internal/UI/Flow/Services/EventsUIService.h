#pragma once

#include "Base/FastName.h"
#include "Reflection/Reflection.h"
#include "UI/Flow/UIFlowService.h"

namespace DAVA
{
class UIControl;

class EventsUIService : public UIFlowService
{
    DAVA_VIRTUAL_REFLECTION(EventsUIService, UIFlowService);

public:
    void Send(UIControl* control, const FastName& event);
};
}