#include "Platform/DeviceInfo.h"

#ifdef __DAVAENGINE_MACOS__

#import <Foundation/NSLocale.h>
#import <Foundation/NSTimeZone.h>
#include "Utils/StringFormat.h"

namespace DAVA
{

String DeviceInfo::GetVersion()
{
	return "Not yet implemented";
}

String DeviceInfo::GetManufacturer()
{
	return "Apple inc.";
}

String DeviceInfo::GetModel()
{
	return "Not yet implemented";
}

String DeviceInfo::GetLocale()
{
	NSLocale *english = [[[NSLocale alloc] initWithLocaleIdentifier:@"en_US"] autorelease];

	NSString* langID = [[NSLocale preferredLanguages] objectAtIndex:0];
	NSString *lang = [english displayNameForKey:NSLocaleLanguageCode value:langID];

	String res = Format("%s (%s)", [langID UTF8String], [lang UTF8String]);
	return res;
}

String DeviceInfo::GetRegion()
{
	NSLocale *english = [[[NSLocale alloc] initWithLocaleIdentifier:@"en_US"] autorelease];

	NSString *countryCode = [[NSLocale currentLocale] objectForKey: NSLocaleCountryCode];
	NSString *country = [english displayNameForKey: NSLocaleCountryCode value: countryCode];

	String res = Format("%s (%s)", [countryCode UTF8String], [country UTF8String]);
	return res;
}

String DeviceInfo::GetTimeZone()
{
	NSTimeZone *localTime = [NSTimeZone systemTimeZone];
    
    String res = Format("%s", [[localTime name] UTF8String]);
	return res;
}

}

#endif
