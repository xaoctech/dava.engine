//
//  DeviceInfo.cpp
//  LevelPerformanceTestiPhone
//
//  Created by Igor Solovey on 3/6/13.
//
//

#include "DeviceInfo.h"

namespace DAVA
{
#ifndef __DAVAENGINE_IPHONE__
    String DeviceInfo::GetDeviceDescription()
    {
        return "Unknown Device";
    }
#endif
}
