#include "Base/BaseTypes.h"
#include "Platform/DPIHelper.h"

#include <AppKit/NSScreen.h>
#include <ApplicationServices/ApplicationServices.h>
#include <Foundation/NSDictionary.h>

namespace DAVA
{
    uint32 DPIHelper::GetScreenDPI()
    {
        
        NSScreen *screen = [NSScreen mainScreen];
        NSDictionary *description = [screen deviceDescription];
        NSSize displayPixelSize = [[description objectForKey:NSDeviceSize] sizeValue];
        CGSize displayPhysicalSize =
            CGDisplayScreenSize([[description objectForKey:@"NSScreenNumber"] unsignedIntValue]);
        
        if(displayPhysicalSize.width == 0)
        {
            return 0;
        }
        return  (displayPixelSize.width / displayPhysicalSize.width) * 25.4f;
    }
	
}