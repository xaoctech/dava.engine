//
// Created by Yury Drazdouski on 11/2/13.
//


#include <GameController/GameController.h>
#import "GamepadManager.h"
#import "GamepadDevice.h"

using namespace DAVA;

@interface GamepadManagerIOS : NSObject

- (id)initWithParent:(GamepadManager *)parent;
- (GamepadDevice *)getCurrentGamepad;
@property(nonatomic, retain) GCController *currentController;

@end


namespace DAVA
{

    GamepadManager::GamepadManager()
    {
        m_gamepadManagerIOS = [[GamepadManagerIOS alloc] initWithParent:this];
    }

    GamepadDevice *GamepadManager::GetCurrentGamepad()
    {
        return [(GamepadManagerIOS *)m_gamepadManagerIOS getCurrentGamepad];
    }
}


@implementation GamepadManagerIOS {
@private
    bool _isListening;
    GamepadManager *_parent;
    RefPtr<GamepadDevice> _currentGamepad;
}

@synthesize currentController = _currentController;

- (id)initWithParent:(GamepadManager *)parent {
    self = [super init];
    if (self) {
        _isListening = NO;
        _parent = parent;
        [self startListening];
    }

    return self;
}

- (void)dealloc {
    [self stopListening];
    self.currentController = nil;
    [super dealloc];
}

- (GamepadDevice *)getCurrentGamepad {
    return _currentGamepad.Get();
}

- (void)startListening {
    if(!_isListening) {
        [self findController];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(gameControllerDidConnect) name:@"GCControllerDidConnectNotification" object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(gameControllerDidDisconnect) name:@"GCControllerDidDisconnectNotification" object:nil];
        _isListening = YES;
    }
}

- (void)stopListening {
    if(_isListening) {
        [[NSNotificationCenter defaultCenter] removeObserver:self];
        _isListening = NO;
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
        _currentGamepad = NULL;
    }
}

- (eDavaGamepadProfile)getProfile
{
    if(_currentController == nil) {
        return eDavaGamepadProfile_None;
    } else {
        return _currentController.extendedGamepad ? eDavaGamepadProfile_ExtendedProfile : eDavaGamepadProfile_SimpleProfile;
    }
}


- (void)findController {
    self.currentController = nil;
    NSArray *controllers = [GCController controllers];
    if(controllers.count) {
        self.currentController = [controllers objectAtIndex:0];
        _currentGamepad = RefPtr<GamepadDevice>(new GamepadDevice(_currentController));
    }
}

@end


