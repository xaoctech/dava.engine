#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_COREV2__)
#if defined(__DAVAENGINE_MACOS__)

namespace DAVA
{
class GamepadDevice;
namespace Private
{
struct MainDispatcherEvent;
class GamepadDeviceImpl final
{
public:
    GamepadDeviceImpl(GamepadDevice* gamepad);

    void Update();

    void HandleGamepadMotion(const MainDispatcherEvent&)
    {
    }
    void HandleGamepadButton(const MainDispatcherEvent&)
    {
    }

    bool HandleGamepadAdded(uint32 id);
    bool HandleGamepadRemoved(uint32 id);

    GamepadDevice* gamepadDevice = nullptr;
};

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_MACOS__
#endif // __DAVAENGINE_COREV2__
