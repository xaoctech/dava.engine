#include "Utils/UTF8Utils.h"
#include "FileSystem/Logger.h"

// This code is identical both for MacOSX and iOS.
#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_MACOS__)

#import <Foundation/Foundation.h>

namespace DAVA 
{

void UTF8Utils::EncodeToWideString(uint8 * string, int32 size, WideString & resultString)
{
	char* buf = new char[size + 1];
	memcpy(buf, string, size);
	buf[size] = 0;

	NSString* nsstring = [[NSString alloc] initWithUTF8String:buf];
	delete[] buf;

	NSStringEncoding encoding = CFStringConvertEncodingToNSStringEncoding(kCFStringEncodingUTF32LE);
	NSData* data = [nsstring dataUsingEncoding:encoding];

	resultString = WideString((wchar_t*)[data bytes], [data length] / sizeof(wchar_t));

	[nsstring release];
}

String UTF8Utils::EncodeToUTF8(const WideString& wstring)
{
	NSStringEncoding encoding = CFStringConvertEncodingToNSStringEncoding(kCFStringEncodingUTF32LE);
	NSString* nsstring = [[NSString alloc]
						  initWithBytes:(const char*)wstring.c_str()
						  length:wstring.length() * sizeof(wchar_t)
						  encoding:encoding];

	String res = [nsstring UTF8String];

	[nsstring release];

	return res;
}

};

#endif
