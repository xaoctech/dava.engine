#include "Input/Private/Mac/GamepadDeviceImplMac.h"

#if defined(__DAVAENGINE_COREV2__)
#if defined(__DAVAENGINE_MACOS__)

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

#endif // __DAVAENGINE_MACOS__
#endif // __DAVAENGINE_COREV2__
