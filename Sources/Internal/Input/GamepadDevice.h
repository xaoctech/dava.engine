#pragma once

#include <bitset>

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "Engine/EngineTypes.h"

namespace DAVA
{
class InputSystem;
namespace Private
{
class GamepadDeviceImpl;
struct MainDispatcherEvent;
}

/**
    \ingroup input
    Class for working with gamepads.
*/
class GamepadDevice final : public BaseObject
{
    friend class InputSystem;
    friend class Private::GamepadDeviceImpl;

public:
    bool IsPresent() const;
    eGamepadProfiles GetProfile() const;

    float32 operator[](eGamepadElements element) const;
    float32 GetElementState(eGamepadElements element) const;

private:
    GamepadDevice(InputSystem* inputSystem_);
    ~GamepadDevice();

    void Update();

    void HandleGamepadAdded(const Private::MainDispatcherEvent& e);
    void HandleGamepadRemoved(const Private::MainDispatcherEvent& e);

    void HandleGamepadMotion(const Private::MainDispatcherEvent& e);
    void HandleGamepadButton(const Private::MainDispatcherEvent& e);

    InputSystem* inputSystem = nullptr;
    std::unique_ptr<Private::GamepadDeviceImpl> impl;
    bool isPresent = false;
    eGamepadProfiles profile = eGamepadProfiles::SIMPLE;

    static const size_t ELEMENT_COUNT = static_cast<size_t>(eGamepadElements::LAST) + 1;
    float32 elementValues[ELEMENT_COUNT];
    int64 elementTimestamps[ELEMENT_COUNT];
    std::bitset<ELEMENT_COUNT> elementChangedMask;
};

inline bool GamepadDevice::IsPresent() const
{
    return isPresent;
}

inline eGamepadProfiles GamepadDevice::GetProfile() const
{
    return profile;
}

inline float32 GamepadDevice::operator[](eGamepadElements element) const
{
    const size_t index = static_cast<size_t>(element);
    return elementValues[index];
}

inline float32 GamepadDevice::GetElementState(eGamepadElements element) const
{
    return (*this)[element];
}

} // namespace DAVA
