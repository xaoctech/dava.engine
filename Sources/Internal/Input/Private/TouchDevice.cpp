#include "Input/TouchDevice.h"

#include "Engine/Engine.h"
#include "Engine/Private/Dispatcher/MainDispatcherEvent.h"
#include "Input/InputSystem.h"
#include "Input/Private/DIElementWrapper.h"

namespace DAVA
{
TouchDevice::TouchDevice(uint32 id)
    : InputDevice(id)
    , inputSystem(GetEngineContext()->inputSystem)
    , clicks{}
    , positions{}
    , nativeTouchIds{}
{
    endFrameConnectionToken = Engine::Instance()->endFrame.Connect(this, &TouchDevice::OnEndFrame);
    Private::EngineBackend::Instance()->InstallEventFilter(this, MakeFunction(this, &TouchDevice::HandleEvent));

    for (eDigitalElementStates& e : clicks)
    {
        e = eDigitalElementStates::RELEASED;
    }
}

TouchDevice::~TouchDevice()
{
    Engine::Instance()->endFrame.Disconnect(endFrameConnectionToken);
    Private::EngineBackend::Instance()->UninstallEventFilter(this);
}

bool TouchDevice::SupportsElement(eInputElements elementId) const
{
    // TODO: honestly check if we support an element with specified id
    // i.e. android can support different amount of touches, iOS supports only up to 5 touches
    return IsTouchInputElement(elementId);
}

eDigitalElementStates TouchDevice::GetDigitalElementState(eInputElements elementId) const
{
    DVASSERT(IsTouchClickElement(elementId));
    return clicks[elementId - eInputElements::TOUCH_FIRST_CLICK];
}

AnalogElementState TouchDevice::GetAnalogElementState(eInputElements elementId) const
{
    DVASSERT(IsTouchPositionElement(elementId));
    return positions[elementId - eInputElements::TOUCH_FIRST_POSITION];
}

bool TouchDevice::HandleEvent(const Private::MainDispatcherEvent& e)
{
    using Private::MainDispatcherEvent;

    if (e.type != MainDispatcherEvent::TOUCH_DOWN && e.type != MainDispatcherEvent::TOUCH_UP && e.type != MainDispatcherEvent::TOUCH_MOVE)
    {
        return false;
    }

    // Create input event
    InputEvent inputEvent;
    inputEvent.window = e.window;
    inputEvent.timestamp = static_cast<float64>(e.timestamp / 1000.0f);
    inputEvent.deviceType = eInputDeviceTypes::TOUCH_SURFACE;
    inputEvent.deviceId = GetId();

    if (e.type == MainDispatcherEvent::TOUCH_DOWN)
    {
        const int touchIndex = GetFirstNonUsedTouchIndex();
        if (touchIndex < 0)
        {
            DVASSERT(false);
            return false;
        }

        // Update digital part

        DIElementWrapper digitalElement(clicks[touchIndex]);
        digitalElement.Press();

        // Update analog part

        AnalogElementState& analogState = positions[touchIndex];
        analogState.x = e.touchEvent.x;
        analogState.y = e.touchEvent.y;

        // Save native touch id to be able to local correct touch when TOUCH_UP event comes

        nativeTouchIds[touchIndex] = e.touchEvent.touchId;

        // Send input event

        inputEvent.digitalState = digitalElement.GetState();
        inputEvent.elementId = static_cast<eInputElements>(eInputElements::TOUCH_FIRST_CLICK + touchIndex);

        inputSystem->DispatchInputEvent(inputEvent);
    }
    else if (e.type == MainDispatcherEvent::TOUCH_UP)
    {
        // Find out index the of touch

        int touchIndex = -1;
        for (int i = 0; i < INPUT_ELEMENTS_TOUCH_CLICK_COUNT; ++i)
        {
            if (nativeTouchIds[i] == e.touchEvent.touchId)
            {
                touchIndex = i;
                break;
            }
        }

        if (touchIndex == -1)
        {
            DVASSERT(false);
            return false;
        }

        // Update digital part

        DIElementWrapper element(clicks[touchIndex]);
        element.Release();

        // Reset native id for this touch

        nativeTouchIds[touchIndex] = 0;

        // Update analog part

        AnalogElementState& analogState = positions[touchIndex];
        analogState.x = e.touchEvent.x;
        analogState.y = e.touchEvent.y;

        // Send input event

        inputEvent.digitalState = element.GetState();
        inputEvent.elementId = static_cast<eInputElements>(eInputElements::TOUCH_FIRST_CLICK + touchIndex);

        inputSystem->DispatchInputEvent(inputEvent);

        // If it's an up event, reset position AFTER sending the input event
        // (so that users can request it during handling and get correct position)
        analogState.x = 0.0f;
        analogState.y = 0.0f;
    }
    else if (e.type == MainDispatcherEvent::TOUCH_MOVE)
    {
        // Find out index the of touch

        int touchIndex = -1;
        for (int i = 0; i < INPUT_ELEMENTS_TOUCH_CLICK_COUNT; ++i)
        {
            if (nativeTouchIds[i] == e.touchEvent.touchId)
            {
                touchIndex = i;
                break;
            }
        }

        if (touchIndex == -1)
        {
            DVASSERT(false);
            return false;
        }

        // Update analog part

        AnalogElementState& analogState = positions[touchIndex];
        analogState.x = e.touchEvent.x;
        analogState.y = e.touchEvent.y;

        // Create and send input event

        inputEvent.analogState = analogState;
        inputEvent.elementId = static_cast<eInputElements>(eInputElements::TOUCH_FIRST_POSITION + touchIndex);

        inputSystem->DispatchInputEvent(inputEvent);
    }

    return true;
}

void TouchDevice::OnEndFrame()
{
    // Promote JustPressed & JustReleased states to Pressed/Released accordingly
    for (DIElementWrapper d : clicks)
    {
        d.OnEndFrame();
    }
}

int TouchDevice::GetFirstNonUsedTouchIndex() const
{
    for (int i = 0; i < INPUT_ELEMENTS_TOUCH_CLICK_COUNT; ++i)
    {
        if ((clicks[i] & eDigitalElementStates::RELEASED) == eDigitalElementStates::RELEASED)
        {
            return i;
        }
    }

    return -1;
}
} // namespace DAVA
