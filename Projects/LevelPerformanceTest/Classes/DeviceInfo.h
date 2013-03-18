//
//  DeviceInfo.h
//  LevelPerformanceTestiPhone
//
//  Created by Igor Solovey on 3/6/13.
//
//

#ifndef __LevelPerformanceTestiPhone__DeviceInfo__
#define __LevelPerformanceTestiPhone__DeviceInfo__

#include "DAVAEngine.h"

namespace DAVA
{
    class DeviceInfo : public Singleton<DeviceInfo>
    {
    public:
        String GetDeviceDescription();
    };
    
}

#endif /* defined(__LevelPerformanceTestiPhone__DeviceInfo__) */
