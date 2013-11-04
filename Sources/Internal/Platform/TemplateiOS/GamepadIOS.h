//
// Created by Yury Drazdouski on 11/2/13.
//


#import <Foundation/Foundation.h>

@class GCController;

@interface GamepadIOS : NSObject

-(id)init;
-(void)startListening;

@property(nonatomic, retain) GCController *currentController;

@end