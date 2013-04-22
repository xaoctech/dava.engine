//
//  Config.h
//  LevelPerformanceTestiPhone
//
//  Created by Igor Solovey on 3/5/13.
//
//

#ifndef LevelPerformanceTestiPhone_Config_h
#define LevelPerformanceTestiPhone_Config_h

#define DATABASE_IP               String("by2-buildmachine.wargaming.net")
#define DATAPASE_PORT               27017

#define DATABASE_NAME               String("LevelPerformanceTests")
#define DATABASE_COLLECTION         String("LevelPerformanceTestsResult")

#if defined (__DAVAENGINE_MACOS__)
#define PLATFORM_NAME           String("MacOS")
#elif defined (__DAVAENGINE_IPHONE__)
#define PLATFORM_NAME           String("iPhone")
#elif defined (__DAVAENGINE_WIN32__)
#define PLATFORM_NAME           String("Win32")
#elif defined (__DAVAENGINE_ANDROID__)
#define PLATFORM_NAME           String("Android")
#else
#define PLATFORM_NAME           String("Unknown")
#endif //PLATFORMS

#define RESULT_TEXTURE_SCALE     0.5f
#define MINFPS_TARGET_SIZE          3.f

#endif
