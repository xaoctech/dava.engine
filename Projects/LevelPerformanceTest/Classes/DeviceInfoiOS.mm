//
//  DeviceInfoiOS.mm
//  TemplateProjectiPhone
//
//  Created by Igor Solovey on 9/19/12.
//  Copyright (c) 2012 DAVA, Inc. All rights reserved.
//

#import "DeviceInfo.h"

#ifdef __DAVAENGINE_IPHONE__
namespace DAVA
{
    String DeviceInfo::GetDeviceDescription()
    {
        NSString * deviceName = [[UIDevice currentDevice] name];
        return String([deviceName cStringUsingEncoding:NSUTF8StringEncoding]);
    }
};
#endif