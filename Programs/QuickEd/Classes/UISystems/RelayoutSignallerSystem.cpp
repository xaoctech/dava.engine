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
    std::for_each(relayoutedControls.begin(), relayoutedControls.end(), [&](DAVA::UIControl* control) { beforeRelayoutedControlRender.Emit(control); });
    relayoutedControls.clear();
}

void RelayoutSignallerSystem::OnControlLayouted(DAVA::UIControl* control)
{
    relayoutedControls.push_back(control);
}
