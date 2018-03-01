#include "InputUtils.h"

#include <DeviceManager/DeviceManager.h>
#include <Engine/Engine.h>
#include <Input/Keyboard.h>
#include <Input/Mouse.h>

DAVA::uint32 InputUtils::GetKeyboardDeviceId()
{
    using namespace DAVA;

    uint32 keyboardId = ~0;
    DeviceManager* deviceManager = GetEngineContext()->deviceManager;
    if (nullptr != deviceManager && nullptr != deviceManager->GetKeyboard())
    {
        keyboardId = deviceManager->GetKeyboard()->GetId();
    }
    return keyboardId;
}

DAVA::uint32 InputUtils::GetMouseDeviceId()
{
    using namespace DAVA;

    uint32 mouseId = ~0;
    DeviceManager* deviceManager = GetEngineContext()->deviceManager;
    if (nullptr != deviceManager && nullptr != deviceManager->GetMouse())
    {
        mouseId = deviceManager->GetMouse()->GetId();
    }
    return mouseId;
}
