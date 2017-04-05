#include "Input/Private/Win32/GamepadDeviceImplWin32.h"

#if defined(__DAVAENGINE_COREV2__)
#if defined(__DAVAENGINE_WIN32__)

namespace DAVA
{
namespace Private
{
GamepadDeviceImpl::GamepadDeviceImpl(GamepadDevice* gamepadDevice)
    : gamepadDevice(gamepadDevice)
{
}

void GamepadDeviceImpl::Update()
{
}

bool GamepadDeviceImpl::HandleGamepadAdded(uint32 /*id*/)
{
    return false;
}

bool GamepadDeviceImpl::HandleGamepadRemoved(uint32 /*id*/)
{
    return false;
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_WIN32__
#endif // __DAVAENGINE_COREV2__
