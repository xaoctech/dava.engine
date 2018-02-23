#pragma once

#include "Base/FastName.h"

namespace DAVA
{
class UIControl;
class UIEventsSystem;

class UIEventProcessorDelegate
{
public:
    virtual ~UIEventProcessorDelegate();
    virtual bool ProcessEvent(UIControl* target, const FastName& event) = 0;

private:
    void SetSystem(UIEventsSystem* sys);
    UIEventsSystem* system = nullptr;
    friend class UIEventsSystem;
};
}