#pragma once

#include "Platform/OpenUDID_apple.h"

@interface OpenUDIDiOS : OpenUDID
{
}

- (void)setDict:(id)dict forPasteboard:(id)pboard;
- (id)getDataForPasteboard:(id)pboard;
- (id)getPasteboardWithName:(NSString*)name shouldCreate:(BOOL)create setPersistent:(BOOL)persistent;

@end
