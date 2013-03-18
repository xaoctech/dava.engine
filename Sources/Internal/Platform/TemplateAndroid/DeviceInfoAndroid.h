#ifndef __FRAMEWORK__DEVICEINFOANDROID__
#define __FRAMEWORK__DEVICEINFOANDROID__

#include "JniExtensions.h"

namespace DAVA
{

class JniDeviceInfo: public JniExtension
{
public:
	JniDeviceInfo();

	String GetVersion();
	String GetManufacturer();
	String GetModel();
	String GetLocale();
	String GetRegion();
	String GetTimeZone();
};

};

#endif /* defined(__FRAMEWORK__DEVICEINFOANDROID__) */
