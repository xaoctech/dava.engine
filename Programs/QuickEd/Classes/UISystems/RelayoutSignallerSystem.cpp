#include "UISystems/RelayoutSignallerSystem.h"

#include <UI/UIControlSystem.h>
#include <UI/Layouts/UILayoutSystem.h>

RelayoutSignallerSystem::RelayoutSignallerSystem()
{
    DAVA::UIControlSystem::Instance()->GetLayoutSystem()->AddListener(this);
}

RelayoutSignallerSystem::~RelayoutSignallerSystem()
{
    DAVA::UIControlSystem::Instance()->GetLayoutSystem()->RemoveListener(this);
}

void RelayoutSignallerSystem::Process(DAVA::float32 elapsedTime)
{
    if (isDirty)
    {
        isDirty = false;
        beforeRelayoutedControlRender.Emit();
    }
}

void RelayoutSignallerSystem::OnControlLayouted(DAVA::UIControl* control)
{
    isDirty = true;
}
