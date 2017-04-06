#if defined(__DAVAENGINE_COREV2__)

#include <algorithm>

#include "Input/GamepadDevice.h"

#include "Debug/DVAssert.h"
#include "Engine/Engine.h"
#include "Engine/Private/Dispatcher/MainDispatcherEvent.h"
#include "Input/InputEvent.h"
#include "Input/InputSystem.h"
#include "Input/Private/DigitalElement.h"
#include "UI/UIEvent.h"

#if defined(__DAVAENGINE_ANDROID__)
#include "Input/Private/Android/GamepadDeviceImplAndroid.h"
#elif defined(__DAVAENGINE_WIN_UAP__)
#include "Input/Private/Win10/GamepadDeviceImplWin10.h"
#elif defined(__DAVAENGINE_WIN32__)
#include "Input/Private/Win32/GamepadDeviceImplWin32.h"
#elif defined(__DAVAENGINE_MACOS__)
#include "Input/Private/Mac/GamepadDeviceImplMac.h"
#elif defined(__DAVAENGINE_IPHONE__)
#include "Input/Private/Ios/GamepadDeviceImplIos.h"
#else
#error "GamepadDevice: unknown platform"
#endif

namespace DAVA
{
GamepadDevice::GamepadDevice(uint32 id)
    : InputDevice(id)
    , inputSystem(GetEngineContext()->inputSystem)
    , impl(new Private::GamepadDeviceImpl(this))
{
    std::fill(std::begin(buttons), std::end(buttons), eDigitalElementStates::NONE);
    std::fill(std::begin(axises), std::end(axises), AnalogElementState());

    endFrameConnectionToken = Engine::Instance()->endFrame.Connect(this, &GamepadDevice::OnEndFrame);
}

GamepadDevice::~GamepadDevice()
{
    Engine::Instance()->endFrame.Disconnect(endFrameConnectionToken);
}

bool GamepadDevice::SupportsElement(eInputElements elementId) const
{
    DVASSERT(IsGamepadAxis(elementId) || IsGamepadButton(elementId));
    return supportedElements[elementId - eInputElements::GAMEPAD_FIRST];
}

eDigitalElementStates GamepadDevice::GetDigitalElementState(eInputElements elementId) const
{
    DVASSERT(IsGamepadButton(elementId));
    return buttons[elementId - eInputElements::GAMEPAD_FIRST_BUTTON];
}

AnalogElementState GamepadDevice::GetAnalogElementState(eInputElements elementId) const
{
    DVASSERT(IsGamepadAxis(elementId));
    return axises[elementId - eInputElements::GAMEPAD_FIRST_AXIS];
}

String GamepadDevice::GetElementStringRepresentation(eInputElements elementId) const
{
    DVASSERT(SupportsElement(elementId));
    return GetInputElementInfo(elementId).name;
}

void GamepadDevice::Update()
{
    impl->Update();

    for (uint32 i = eInputElements::GAMEPAD_FIRST_BUTTON; i <= eInputElements::GAMEPAD_LAST_BUTTON; ++i)
    {
        uint32 index = i - eInputElements::GAMEPAD_FIRST_BUTTON;
        if (buttonChangedMask[index])
        {
            InputEvent inputEvent;
            inputEvent.window = nullptr;
            inputEvent.deviceType = eInputDeviceTypes::CLASS_GAMEPAD;
            inputEvent.deviceId = GetId();
            inputEvent.elementId = static_cast<eInputElements>(i);
            inputEvent.digitalState = buttons[index];
            inputSystem->DispatchInputEvent(inputEvent);
        }
    }
    buttonChangedMask.reset();

    for (uint32 i = eInputElements::GAMEPAD_FIRST_AXIS; i <= eInputElements::GAMEPAD_LAST_AXIS; ++i)
    {
        uint32 index = i - eInputElements::GAMEPAD_FIRST_AXIS;
        if (axisChangedMask[index])
        {
            InputEvent inputEvent;
            inputEvent.window = nullptr;
            inputEvent.deviceType = eInputDeviceTypes::CLASS_GAMEPAD;
            inputEvent.deviceId = GetId();
            inputEvent.elementId = static_cast<eInputElements>(i);
            inputEvent.analogState = axises[index];
            inputSystem->DispatchInputEvent(inputEvent);
        }
    }
    axisChangedMask.reset();
}

void GamepadDevice::OnEndFrame()
{
    for (eDigitalElementStates& d : buttons)
    {
        Private::DigitalInputElement(d).OnEndFrame();
    }
}

void GamepadDevice::HandleGamepadAdded(const Private::MainDispatcherEvent& e)
{
    uint32 deviceId = e.gamepadEvent.deviceId;
    impl->HandleGamepadAdded(deviceId);
}

void GamepadDevice::HandleGamepadRemoved(const Private::MainDispatcherEvent& e)
{
    uint32 deviceId = e.gamepadEvent.deviceId;
    impl->HandleGamepadRemoved(deviceId);
}

void GamepadDevice::HandleGamepadMotion(const Private::MainDispatcherEvent& e)
{
    impl->HandleGamepadMotion(e);
}

void GamepadDevice::HandleGamepadButton(const Private::MainDispatcherEvent& e)
{
    impl->HandleGamepadButton(e);
}

void GamepadDevice::HandleButtonPress(eInputElements element, bool pressed)
{
    uint32 index = element - eInputElements::GAMEPAD_FIRST_BUTTON;
    Private::DigitalInputElement di(buttons[index]);
    if (di.IsPressed() != pressed)
    {
        pressed ? di.Press() : di.Release();
        buttonChangedMask.set(index);
    }
}

void GamepadDevice::HandleAxisMovement(eInputElements element, float32 newValue)
{
    uint32 index = element - eInputElements::GAMEPAD_FIRST_AXIS;
    if (newValue != axises[index].x)
    {
        axises[index].x = newValue;
        axisChangedMask.set(index);
    }
}

} // namespace DAVA

#else // __DAVAENGINE_COREV2__

#include "GamepadDevice.h"

namespace DAVA
{
GamepadDevice::GamepadDevice()
{
    Reset();

    InitInternal();
}

void GamepadDevice::Reset()
{
    isAvailable = false;
    profile = GAMEPAD_PROFILE_EXTENDED;
    Memset(elementValues, 0, sizeof(float32) * GAMEPAD_ELEMENT_COUNT);
}

#if defined(__DAVAENGINE_WINDOWS__) || defined(__DAVAENGINE_MACOS__)
void GamepadDevice::InitInternal()
{
}
#endif

#if defined(__DAVAENGINE_ANDROID__)
void GamepadDevice::InitInternal()
{
    Memset(keyTranslator, INVALID_DAVAKEY, MAX_TRANSLATOR_KEYS);
    Memset(axisTranslator, INVALID_DAVAKEY, MAX_TRANSLATOR_KEYS);

    keyTranslator[0x60] = static_cast<uint8>(GAMEPAD_ELEMENT_BUTTON_A); //BUTTON_A
    keyTranslator[0x61] = static_cast<uint8>(GAMEPAD_ELEMENT_BUTTON_B); //BUTTON_B
    keyTranslator[0x63] = static_cast<uint8>(GAMEPAD_ELEMENT_BUTTON_X); //BUTTON_X
    keyTranslator[0x64] = static_cast<uint8>(GAMEPAD_ELEMENT_BUTTON_Y); //BUTTON_Y

    keyTranslator[0x66] = static_cast<uint8>(GAMEPAD_ELEMENT_BUTTON_LS); //BUTTON_L1
    keyTranslator[0x67] = static_cast<uint8>(GAMEPAD_ELEMENT_BUTTON_RS); //BUTTON_R1
    keyTranslator[0x68] = static_cast<uint8>(GAMEPAD_ELEMENT_LT); //BUTTON_L2
    keyTranslator[0x69] = static_cast<uint8>(GAMEPAD_ELEMENT_RT); //BUTTON_R2

    keyTranslator[0x13] = static_cast<uint8>(GAMEPAD_ELEMENT_DPAD_Y); //DPAD_UP
    keyTranslator[0x14] = static_cast<uint8>(GAMEPAD_ELEMENT_DPAD_Y); //DPAD_DOWN
    keyTranslator[0x15] = static_cast<uint8>(GAMEPAD_ELEMENT_DPAD_X); //DPAD_LEFT
    keyTranslator[0x16] = static_cast<uint8>(GAMEPAD_ELEMENT_DPAD_X); //DPAD_RIGHT

    axisTranslator[17] = static_cast<uint8>(GAMEPAD_ELEMENT_LT); //AXIS_LTRIGGER
    axisTranslator[18] = static_cast<uint8>(GAMEPAD_ELEMENT_RT); //AXIS_RTRIGGER
    axisTranslator[23] = static_cast<uint8>(GAMEPAD_ELEMENT_LT); //AXIS_BRAKE
    axisTranslator[22] = static_cast<uint8>(GAMEPAD_ELEMENT_RT); //AXIS_GAS

    axisTranslator[0] = static_cast<uint8>(GAMEPAD_ELEMENT_AXIS_LX); //AXIS_X
    axisTranslator[1] = static_cast<uint8>(GAMEPAD_ELEMENT_AXIS_LY); //AXIS_Y
    axisTranslator[11] = static_cast<uint8>(GAMEPAD_ELEMENT_AXIS_RX); //AXIS_Z
    axisTranslator[14] = static_cast<uint8>(GAMEPAD_ELEMENT_AXIS_RY); //AXIS_RZ
    axisTranslator[12] = static_cast<uint8>(GAMEPAD_ELEMENT_AXIS_RX); //AXIS_RX
    axisTranslator[13] = static_cast<uint8>(GAMEPAD_ELEMENT_AXIS_RY); //AXIS_RY
}
#endif
}

#endif // !__DAVAENGINE_COREV2__
