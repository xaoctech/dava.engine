#include <algorithm>

#include "Input/GamepadDevice.h"

#include "Engine/Private/Dispatcher/MainDispatcherEvent.h"
#include "Input/InputSystem.h"
#include "UI/UIEvent.h"

#if defined(__DAVAENGINE_ANDROID__)
#include "Input/Private/Android/GamepadDeviceImplAndroid.h"
#elif defined(__DAVAENGINE_WIN_UAP__)
#include "Input/Private/Win10/GamepadDeviceImplWin10.h"
#elif defined(__DAVAENGINE_IPHONE__)
#include "Input/Private/Ios/GamepadDeviceImplIos.h"
#else
#include "Input/Private/GamepadDeviceImplStub.h"
#endif

namespace DAVA
{
GamepadDevice::GamepadDevice(InputSystem* inputSystem_)
    : inputSystem(inputSystem_)
    , impl(new Private::GamepadDeviceImpl(this))
{
    std::fill(std::begin(elementValues), std::end(elementValues), float32(0));
    std::fill(std::begin(elementTimestamps), std::end(elementTimestamps), uint64(0));
}

GamepadDevice::~GamepadDevice() = default;

void GamepadDevice::Update()
{
    if (isPresent)
    {
        impl->Update();
        for (size_t i = 0; i < ELEMENT_COUNT; ++i)
        {
            if (elementChangedMask[i])
            {
                UIEvent uie;
                uie.element = static_cast<eGamepadElements>(i);
                uie.physPoint.x = elementValues[i];
                uie.point.x = elementValues[i];
                uie.phase = UIEvent::Phase::JOYSTICK;
                uie.device = eInputDevices::GAMEPAD;
                uie.timestamp = elementTimestamps[i] / 1000.0;

                inputSystem->HandleInputEvent(&uie);
            }
        }
        elementChangedMask.reset();
    }
}

void GamepadDevice::HandleGamepadAdded(const Private::MainDispatcherEvent& e)
{
    uint32 deviceId = e.gamepadEvent.deviceId;
    isPresent = impl->HandleGamepadAdded(deviceId);
}

void GamepadDevice::HandleGamepadRemoved(const Private::MainDispatcherEvent& e)
{
    uint32 deviceId = e.gamepadEvent.deviceId;
    isPresent = impl->HandleGamepadRemoved(deviceId);
}

void GamepadDevice::HandleGamepadMotion(const Private::MainDispatcherEvent& e)
{
    impl->HandleGamepadMotion(e);
}

void GamepadDevice::HandleGamepadButton(const Private::MainDispatcherEvent& e)
{
    impl->HandleGamepadButton(e);
}

} // namespace DAVA
