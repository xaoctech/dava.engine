#include "Base/BaseTypes.h"
#include "Core/Core.h"


#if defined(__DAVAENGINE_IPHONE__)

#include <UIKit/UIKit.h>
#include <UIKit/UIDevice.h>

namespace DAVA
{
    uint32 Core::GetScreenDPI()
    {
        //due to magnificent api of ios the only way of determination of dpi is hardcode
        CGRect screenRect = [[UIScreen mainScreen] bounds];
        CGFloat screenWidth = screenRect.size.width;
        CGFloat screenHeight = screenRect.size.height;
        
         //width and height could be swapped according orientation
        uint32 minRes = screenWidth > screenHeight ? screenHeight : screenWidth;
        uint32 retDPI = 0; // unknown device
        switch (minRes)
        {
            case 320:
                retDPI = 163;// iphone3gs
                break;
            case 640:
                retDPI = 326;// iphone4/4s/5/ipod touch 4/5
                break;
            case 768:// ipad1/2/mini - need more attention
            {
                NSString *name = [[UIDevice currentDevice] name];
                if([name rangeOfString:@"mini"].location != NSNotFound)// ipad mini?
                {
                    retDPI = 163;
                }
                else
                {
                    retDPI = 132; // ipad 1/2
                }
            }
                break;
            case 1536:
                retDPI = 264;// ipad3/4
                break;
            default:
                break;
        }
       
        
        return retDPI;
    }
	
}



#endif // #if defined(__DAVAENGINE_IPHONE__)
