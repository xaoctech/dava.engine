#include "Input/Private/Ios/GamepadDeviceImplIos.h"

#if defined(__DAVAENGINE_COREV2__)
#if defined(__DAVAENGINE_IPHONE__)

#import <GameController/GameController.h>

#include "Input/GamepadDevice.h"
#include "Input/InputElements.h"
#include "Input/Private/DigitalElement.h"

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
    if ([controller extendedGamepad])
    {
        GCExtendedGamepad* gamepad = [controller extendedGamepad];
        ReadExtendedGamepadElements(gamepad);
    }
    else if ([controller gamepad])
    {
        GCGamepad* gamepad = [controller gamepad];
        ReadGamepadElements(gamepad);
    }
}

void GamepadDeviceImpl::ReadExtendedGamepadElements(GCExtendedGamepad* gamepad)
{
    gamepadDevice->HandleButtonPress(eInputElements::GAMEPAD_A, gamepad.buttonA.isPressed);
    gamepadDevice->HandleButtonPress(eInputElements::GAMEPAD_B, gamepad.buttonB.isPressed);
    gamepadDevice->HandleButtonPress(eInputElements::GAMEPAD_X, gamepad.buttonX.isPressed);
    gamepadDevice->HandleButtonPress(eInputElements::GAMEPAD_Y, gamepad.buttonY.isPressed);
    gamepadDevice->HandleButtonPress(eInputElements::GAMEPAD_LSHOUDER, gamepad.leftShoulder.isPressed);
    gamepadDevice->HandleButtonPress(eInputElements::GAMEPAD_RSHOUDER, gamepad.rightShoulder.isPressed);
    gamepadDevice->HandleButtonPress(eInputElements::GAMEPAD_DPAD_LEFT, gamepad.dpad.left.isPressed);
    gamepadDevice->HandleButtonPress(eInputElements::GAMEPAD_DPAD_RIGHT, gamepad.dpad.right.isPressed);
    gamepadDevice->HandleButtonPress(eInputElements::GAMEPAD_DPAD_UP, gamepad.dpad.up.isPressed);
    gamepadDevice->HandleButtonPress(eInputElements::GAMEPAD_DPAD_DOWN, gamepad.dpad.down.isPressed);

    gamepadDevice->HandleAxisMovement(eInputElements::GAMEPAD_LTHUMB_X, gamepad.leftThumbstick.xAxis.value);
    gamepadDevice->HandleAxisMovement(eInputElements::GAMEPAD_LTHUMB_Y, gamepad.leftThumbstick.yAxis.value);
    gamepadDevice->HandleAxisMovement(eInputElements::GAMEPAD_RTHUMB_X, gamepad.rightThumbstick.xAxis.value);
    gamepadDevice->HandleAxisMovement(eInputElements::GAMEPAD_RTHUMB_Y, gamepad.rightThumbstick.yAxis.value);
    gamepadDevice->HandleAxisMovement(eInputElements::GAMEPAD_LTRIGGER, gamepad.leftTrigger.value);
    gamepadDevice->HandleAxisMovement(eInputElements::GAMEPAD_RTRIGGER, gamepad.rightTrigger.value);
}

void GamepadDeviceImpl::ReadGamepadElements(GCGamepad* gamepad)
{
    gamepadDevice->HandleButtonPress(eInputElements::GAMEPAD_A, gamepad.buttonA.isPressed);
    gamepadDevice->HandleButtonPress(eInputElements::GAMEPAD_B, gamepad.buttonB.isPressed);
    gamepadDevice->HandleButtonPress(eInputElements::GAMEPAD_X, gamepad.buttonX.isPressed);
    gamepadDevice->HandleButtonPress(eInputElements::GAMEPAD_Y, gamepad.buttonY.isPressed);
    gamepadDevice->HandleButtonPress(eInputElements::GAMEPAD_LSHOUDER, gamepad.leftShoulder.isPressed);
    gamepadDevice->HandleButtonPress(eInputElements::GAMEPAD_RSHOUDER, gamepad.rightShoulder.isPressed);
    gamepadDevice->HandleButtonPress(eInputElements::GAMEPAD_DPAD_LEFT, gamepad.dpad.left.isPressed);
    gamepadDevice->HandleButtonPress(eInputElements::GAMEPAD_DPAD_RIGHT, gamepad.dpad.right.isPressed);
    gamepadDevice->HandleButtonPress(eInputElements::GAMEPAD_DPAD_UP, gamepad.dpad.up.isPressed);
    gamepadDevice->HandleButtonPress(eInputElements::GAMEPAD_DPAD_DOWN, gamepad.dpad.down.isPressed);
}

bool GamepadDeviceImpl::HandleGamepadAdded(uint32 /*id*/)
{
    if (controller == nullptr)
    {
        NSArray<GCController*>* controllers = [GCController controllers];
        if ([controllers count] != 0)
        {
            controller = (__bridge GCController*)CFBridgingRetain([controllers objectAtIndex:0]);
            gamepadDevice->profile = [controller extendedGamepad] != nil ? eGamepadProfiles::EXTENDED : eGamepadProfiles::SIMPLE;
        }
    }
    return controller != nullptr;
}

bool GamepadDeviceImpl::HandleGamepadRemoved(uint32 id)
{
    bool removed = true;
    for (GCController* c in [GCController controllers])
    {
        if (c == controller)
        {
            removed = false;
            break;
        }
    }
    if (removed && controller != nullptr)
    {
        CFBridgingRelease(controller);
        controller = nullptr;
    }
    return controller != nullptr;
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_IPHONE__
#endif // __DAVAENGINE_COREV2__
