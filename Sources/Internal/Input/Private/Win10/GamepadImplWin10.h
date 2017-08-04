#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_WIN_UAP__)

namespace DAVA
{
class Gamepad;
namespace Private
{
struct MainDispatcherEvent;
class GamepadImpl final
{
public:
    GamepadImpl(Gamepad* gamepad);

    void Update();

    void HandleGamepadMotion(const MainDispatcherEvent&)
    {
    }
    void HandleGamepadButton(const MainDispatcherEvent&)
    {
    }

    bool HandleGamepadAdded(uint32 id);
    bool HandleGamepadRemoved(uint32 id);

    void DetermineSupportedElements();

    Gamepad* gamepadDevice = nullptr;
    ::Windows::Gaming::Input::Gamepad ^ gamepad = nullptr;
};

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_WIN_UAP__
