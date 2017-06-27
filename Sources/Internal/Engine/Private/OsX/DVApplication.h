#pragma once

#import <AppKit/NSApplication.h>

@interface DVApplication : NSApplication

- (void)sendEvent:(NSEvent*)event;

@end
