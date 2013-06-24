#ifndef __FRAMEWORK__DEVICEINFO__
#define __FRAMEWORK__DEVICEINFO__

#include "Base/BaseTypes.h"

namespace DAVA
{

class DeviceInfo
{
public:
	enum ePlatform
	{
		PLATFORM_MACOS = 0,
		PLATFORM_IOS,
		PLATFORM_IOS_SIMULATOR,
		PLATFORM_ANDROID,
		PLATFORM_WIN32,
		PLATFORM_UNKNOWN,
		PLATFORMS_COUNT
	};

	static ePlatform GetPlatform();
	static String GetPlatformString();
	static String GetVersion();
	static String GetManufacturer();
	static String GetModel();
	static String GetLocale();
	static String GetRegion();
	static String GetTimeZone();
    static String GetUDID();
};

};
#endif /* defined(__FRAMEWORK__DEVICEINFO__) */
