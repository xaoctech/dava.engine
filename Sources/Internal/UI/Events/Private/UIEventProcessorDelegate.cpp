#include "UI/Events/UIEventProcessorDelegate.h"
#include "UI/Events/UIEventsSystem.h"
#include "UI/UIControl.h"

namespace DAVA
{
UIEventProcessorDelegate::~UIEventProcessorDelegate()
{
    if (system)
    {
        system->RemoveProcessor(this);
    }
}

void UIEventProcessorDelegate::SetSystem(UIEventsSystem* sys)
{
    system = sys;
}
}