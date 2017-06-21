#pragma once

#include "Base/BaseTypes.h"
#include "Platform/OpenUDIDApple.h"

#if defined(__DAVAENGINE_MACOS__)

#import <Foundation/Foundation.h>

@interface OpenUDIDMac : OpenUDID
{
}

- (void)setDict:(id)dict forPasteboard:(id)pboard;
- (id)getDataForPasteboard:(id)pboard;
- (id)getPasteboardWithName:(NSString*)name shouldCreate:(BOOL)create setPersistent:(BOOL)persistent;

@end


#endif // #if defined(__DAVAENGINE_MACOS__)
