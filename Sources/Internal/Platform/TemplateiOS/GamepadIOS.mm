//
// Created by Yury Drazdouski on 11/2/13.
//


#import "GamepadIOS.h"
#import <GameController/GameController.h>
#include "DAVAEngine.h"
#include "Input/InputSystem.h"
#include "UIEvent.h"

using namespace std;
using namespace DAVA;

DAVA::UIEvent createJoystickEvent(int32 eventId, float64 time, float32 x, float32 y = 0.0f);

@implementation GamepadIOS {
@private
    bool isListening;
}

@synthesize currentController = _currentController;

- (id)init {
    self = [super init];
    if (self) {
        isListening = NO;
    }

    return self;
}

- (void)dealloc {
    [self stopListening];
    self.currentController = nil;
    [super dealloc];
}


- (void)startListening {
    if(!isListening) {
        [self findController];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(gameControllerDidConnect) name:@"GCControllerDidConnectNotification" object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(gameControllerDidDisconnect) name:@"GCControllerDidDisconnectNotification" object:nil];
        isListening = YES;
    }
}

- (void)stopListening {
    if(isListening) {
        [[NSNotificationCenter defaultCenter] removeObserver:self];
    }
}

- (void)findController {
    NSArray *controllers = [GCController controllers];
    if(controllers.count) {
        self.currentController = [controllers objectAtIndex:0];
        if(_currentController.extendedGamepad) {
            _currentController.extendedGamepad.valueChangedHandler = ^(GCExtendedGamepad *gamepad, GCControllerElement *element) {
                [self sendExtendedGamepadState];
            };
        } else {
            _currentController.gamepad.valueChangedHandler = ^(GCGamepad *gamepad, GCControllerElement *element) {
                [self sendGamepadState];
            };
        }
    }
}

- (void)gameControllerDidConnect {
    [self findController];
}

- (void)gameControllerDidDisconnect {
    if(_currentController) {
        _currentController.gamepad.valueChangedHandler = nil;
        _currentController.extendedGamepad.valueChangedHandler = nil;
        _currentController = nil;
    }
}

-(void)sendGamepadState {
    float64 time = 0.0;
    UIEvent event;

    event = createJoystickEvent(UIEvent::GAMEPAD_LSHOULDER, time,
            _currentController.gamepad.leftShoulder.value);
    DAVA::InputSystem::Instance()->ProcessInputEvent(&event);

    event = createJoystickEvent(UIEvent::GAMEPAD_RSHOULDER, time,
            _currentController.gamepad.rightShoulder.value);
    DAVA::InputSystem::Instance()->ProcessInputEvent(&event);

    event = createJoystickEvent(UIEvent::GAMEPAD_DPAD, time,
            _currentController.gamepad.dpad.xAxis.value,
            _currentController.gamepad.dpad.yAxis.value);
    DAVA::InputSystem::Instance()->ProcessInputEvent(&event);

    event = createJoystickEvent(UIEvent::GAMEPAD_BUTTON_A, time, _currentController.gamepad.buttonA.value);
    DAVA::InputSystem::Instance()->ProcessInputEvent(&event);

    event = createJoystickEvent(UIEvent::GAMEPAD_BUTTON_B, time, _currentController.gamepad.buttonB.value);
    DAVA::InputSystem::Instance()->ProcessInputEvent(&event);

    event = createJoystickEvent(UIEvent::GAMEPAD_BUTTON_X, time, _currentController.gamepad.buttonX.value);
    DAVA::InputSystem::Instance()->ProcessInputEvent(&event);

    event = createJoystickEvent(UIEvent::GAMEPAD_BUTTON_Y, time, _currentController.gamepad.buttonY.value);
    DAVA::InputSystem::Instance()->ProcessInputEvent(&event);
}

-(void)sendExtendedGamepadState {
    UIEvent event;
    float64 time = 0.0;

    event = createJoystickEvent(UIEvent::GAMEPAD_LSHOULDER, time,
            _currentController.extendedGamepad.leftShoulder.value);
    DAVA::InputSystem::Instance()->ProcessInputEvent(&event);

    event = createJoystickEvent(UIEvent::GAMEPAD_RSHOULDER, time,
            _currentController.extendedGamepad.rightShoulder.value);
    DAVA::InputSystem::Instance()->ProcessInputEvent(&event);

    event = createJoystickEvent(UIEvent::GAMEPAD_DPAD, time,
            _currentController.extendedGamepad.dpad.xAxis.value,
            _currentController.extendedGamepad.dpad.yAxis.value);
    DAVA::InputSystem::Instance()->ProcessInputEvent(&event);

    event = createJoystickEvent(UIEvent::GAMEPAD_LTHUMBSTICK, time,
            _currentController.extendedGamepad.leftThumbstick.xAxis.value,
            _currentController.extendedGamepad.leftThumbstick.yAxis.value);
    DAVA::InputSystem::Instance()->ProcessInputEvent(&event);

    event = createJoystickEvent(UIEvent::GAMEPAD_RTHUMBSTICK, time,
            _currentController.extendedGamepad.rightThumbstick.xAxis.value,
            _currentController.extendedGamepad.rightThumbstick.yAxis.value);
    DAVA::InputSystem::Instance()->ProcessInputEvent(&event);

    event = createJoystickEvent(UIEvent::GAMEPAD_BUTTON_A, time, _currentController.extendedGamepad.buttonA.value);
    DAVA::InputSystem::Instance()->ProcessInputEvent(&event);

    event = createJoystickEvent(UIEvent::GAMEPAD_BUTTON_B, time, _currentController.extendedGamepad.buttonB.value);
    DAVA::InputSystem::Instance()->ProcessInputEvent(&event);

    event = createJoystickEvent(UIEvent::GAMEPAD_BUTTON_X, time, _currentController.extendedGamepad.buttonX.value);
    DAVA::InputSystem::Instance()->ProcessInputEvent(&event);

    event = createJoystickEvent(UIEvent::GAMEPAD_BUTTON_Y, time, _currentController.extendedGamepad.buttonY.value);
    DAVA::InputSystem::Instance()->ProcessInputEvent(&event);
}

@end


DAVA::UIEvent createJoystickEvent(int32 eventId, float64 time, float32 x, float32 y) {
    DAVA::UIEvent newEvent;
    newEvent.tid = eventId;
    newEvent.physPoint.x = x;
    newEvent.physPoint.y = y;
    newEvent.point.x = x;
    newEvent.point.y = y;
    newEvent.phase = UIEvent::PHASE_JOYSTICK;
    newEvent.tapCount = 1;
    newEvent.timestamp = time;
    return newEvent;
}
