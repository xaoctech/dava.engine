#include "Input/Private/Linux/GamepadImplLinux.h"

#if defined(__DAVAENGINE_LINUX__)

namespace DAVA
{
namespace Private
{
GamepadImpl::GamepadImpl(Gamepad* gamepadDevice)
    : gamepadDevice(gamepadDevice)
{
}

void GamepadImpl::Update()
{
}

bool GamepadImpl::HandleGamepadAdded(uint32 /*id*/)
{
    return false;
}

bool GamepadImpl::HandleGamepadRemoved(uint32 /*id*/)
{
    return false;
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_LINUX__
