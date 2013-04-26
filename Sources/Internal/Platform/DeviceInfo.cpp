#include "DeviceInfo.h"

#if defined(__DAVAENGINE_IPHONE__)
#include "TargetConditionals.h"
#endif

namespace DAVA
{

DeviceInfo::ePlatform DeviceInfo::GetPlatform()
{
	ePlatform platform = PLATFORM_UNKNOWN;

#if defined(__DAVAENGINE_MACOS__)
	platform = PLATFORM_MACOS;

#elif defined(__DAVAENGINE_IPHONE__)
	platform = PLATFORM_IOS;
	#if defined(TARGET_IPHONE_SIMULATOR) && TARGET_IPHONE_SIMULATOR == 1
		platform = PLATFORM_IOS_SIMULATOR;
	#endif

#elif defined(__DAVAENGINE_ANDROID__)
	platform = PLATFORM_ANDROID;

#elif defined(__DAVAENGINE_WIN32__)
	platform = PLATFORM_WIN32;
#endif

	return platform;
}

String DeviceInfo::GetPlatformString()
{
	String res = "";

	switch (GetPlatform())
	{
		case PLATFORM_IOS:
			res = "iOS";
			break;

		case PLATFORM_IOS_SIMULATOR:
			res = "iOS Simulator";
			break;

		case PLATFORM_MACOS:
			res = "MacOS";
			break;

		case PLATFORM_ANDROID:
			res = "Android";
			break;

		case PLATFORM_WIN32:
			res = "Win32";
			break;

		default:
			res = "Unknown";
			break;
	}

	return res;
}

}