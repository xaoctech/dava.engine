#include "DLCManager/Private/EnableDebugProfiler.h"
#include "Engine/Engine.h"
#include "Input/InputSystem.h"
#include "Input/Mouse.h"
#include "DeviceManager/DeviceManager.h"

namespace DAVA
{
EnableDebugProfiler::EnableDebugProfiler()
    : history(8)
{
    if (GetEngineContext()->inputSystem != nullptr)
    {
        AddListenerOnMouseAndTouch();
    }
    else
    {
        Engine::Instance()->windowCreated.Connect(this, &EnableDebugProfiler::OnWindowCreated);
    }
}

EnableDebugProfiler::~EnableDebugProfiler()
{
    Engine::Instance()->windowCreated.Disconnect(this);

    InputSystem* inputSys = GetEngineContext()->inputSystem;
    if (inputSys)
    {
        inputSys->RemoveHandler(handlerToken);
    }
    handlerToken = 0;
}

void EnableDebugProfiler::AddListenerOnMouseAndTouch()
{
    InputSystem* inputSystem = GetEngineContext()->inputSystem;
    if (inputSystem)
    {
        eInputDevices deviceMask = eInputDeviceTypes::MOUSE | eInputDeviceTypes::TOUCH_SURFACE;
        handlerToken = inputSystem->AddHandler(deviceMask, Function<bool(const InputEvent&)>(this, &EnableDebugProfiler::OnMouseOrTouch));
    }
}

void EnableDebugProfiler::OnWindowCreated(Window*)
{
    AddListenerOnMouseAndTouch();
}

bool EnableDebugProfiler::OnMouseOrTouch(const InputEvent& ev)
{
    if (ev.deviceType == eInputDeviceTypes::MOUSE
        && ev.elementId == MOUSE_FIRST
        && ev.digitalState.IsJustReleased())
    {
        InputEvent& inEvent = history.next();
        inEvent = ev;
        DeviceManager* devManager = GetEngineContext()->deviceManager;
        Mouse* mouse = devManager->GetMouse();
        inEvent.analogState = mouse->GetPosition();
    }
    else if (ev.deviceType == eInputDeviceTypes::TOUCH_SURFACE
             && ev.elementId == TOUCH_CLICK0
             && ev.digitalState.IsJustReleased())
    {
        InputEvent& inEvent = history.next();
        inEvent = ev;
        DeviceManager* devManager = GetEngineContext()->deviceManager;
        TouchScreen* touchScreen = devManager->GetTouchScreen();
        inEvent.analogState = touchScreen->GetTouchPositionByIndex(0);
    }
    else
    {
        return false;
    }

    //this is a screen of your android device, tap inside squares in correct order
    // 0, 0 ------------------------------------------------------------> 1024
    // | +-------------------------------------------------------------+
    // | |  1  |                    |  3   |                     |  5  |
    // | |     |                    |      |                     |     |
    // | +-----+                    +------+                     +-----+
    // | |                                                             |
    // | |                                                             |
    // | |                                                             |
    // | |                                                             |
    // | |                                                             |
    // | |                                                             |
    // | |                                                             |
    // | |                                                             |
    // | +-----+                    +------+                     +-----+
    // | |  4  |                    |  2   |                     |  6  |
    // | |     |                    |      |                     |     |
    // | +-------------------------------------------------------------+
    // V
    // 768

    const float32 horiz_size = 1024.f / 8;
    const float32 vert_size = 768.f / 4;

    const float32 first_center_x = horiz_size / 2.f;
    const float32 first_center_y = vert_size / 2.f;

    const float radius = sqrt(first_center_x * first_center_x + first_center_y * first_center_y);

    // top row
    const Vector2 first(horiz_size * 0 + first_center_x, vert_size * 0 + first_center_y);
    const Vector2 third(horiz_size * 4 + first_center_x, vert_size * 0 + first_center_y);
    const Vector2 five_(horiz_size * 7 + first_center_x, vert_size * 0 + first_center_y);

    // bottom row
    const Vector2 four_(horiz_size * 0 + first_center_x, vert_size * 3 + first_center_y);
    const Vector2 tow__(horiz_size * 4 + first_center_x, vert_size * 3 + first_center_y);
    const Vector2 six__(horiz_size * 7 + first_center_x, vert_size * 3 + first_center_y);

    const std::array<Vector2, 6> positions{ first, tow__, third, four_, five_, six__ };

    // check last 6 events in history match
    bool match = true;
    for (uint32 i = 0; i < 6; ++i)
    {
        auto it = history.rbegin() + i;
        Vector2 historyVec(it->analogState.x, it->analogState.y);
        auto secIt = positions.rbegin() + i;
        Vector2 needPos(*secIt);
        Vector2 delta = needPos - historyVec;
        if (delta.Length() > radius)
        {
            match = false;
            break;
        }
    }

    if (match)
    {
        debugGestureMatch.Emit();
    }

    return match; // let other input listeners handle this event if not match
}

} // end namespace DAVA