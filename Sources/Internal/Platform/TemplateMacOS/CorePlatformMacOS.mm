#include "Base/BaseTypes.h"
#include "Core/Core.h"


#if defined(__DAVAENGINE_MACOS__)

#include <AppKit/NSScreen.h>
#include <ApplicationServices/ApplicationServices.h>
#include <Foundation/NSDictionary.h>

namespace DAVA
{
    uint32 Core::GetScreenDPI()
    {
        
        NSScreen *screen = [NSScreen mainScreen];
        NSDictionary *description = [screen deviceDescription];
        NSSize displayPixelSize = [[description objectForKey:NSDeviceSize] sizeValue];
        CGSize displayPhysicalSize =
            CGDisplayScreenSize([[description objectForKey:@"NSScreenNumber"] unsignedIntValue]);
        
        
        return  (displayPixelSize.width / displayPhysicalSize.width) * 25.4f;
    }
	
}



#endif // #if defined(__DAVAENGINE_MACOS__)
