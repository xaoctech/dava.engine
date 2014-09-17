//
//  NSStringUtils.m
//  Framework
//
//  Created by Aleksei Kanash on 9/16/14.
//
//

#import "NSStringUtils.h"

@implementation NSStringUtils

+ (NSString *) NSStringFromWideString:(const DAVA::WideString &)str
{
    
    NSStringEncoding encoding = CFStringConvertEncodingToNSStringEncoding(kCFStringEncodingUTF32LE);
	NSString *nsstring = [[NSString alloc]
                   initWithBytes:(const char*)str.c_str()
                   length:str.length() * sizeof(wchar_t)
                   encoding:encoding];
    return nsstring;
    
}

@end
