//
//  MailSenderMacOS.cpp
//  Framework
//
//  Created by Denis Bespalov on 2/19/13.
//
//

#include "Network/MailSender.h"

#if defined(__DAVAENGINE_IPHONE__)
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#elif defined(__DAVAENGINE_MACOS__)
#import <Foundation/Foundation.h>
#import <AppKit/NSWorkspace.h>
#endif

namespace DAVA
{
#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_MACOS__)
	bool MailSender::SendEmail(const String &email, const String &subject, const String &messageText)
	{
		// Convert input values into NSString
		NSString* msgEmail = [NSString stringWithUTF8String:email.c_str()];
		NSString* msgSubj = [NSString stringWithUTF8String:subject.c_str()];
		NSString* msgBody = [NSString stringWithUTF8String:messageText.c_str()];
		// Build mailto string
		NSString* mailtoString = [NSString stringWithFormat:@"mailto:?to=%@&subject=%@&body=%@",
									msgEmail, msgSubj, msgBody];
		// Buiild corect web string without special charachters
		NSString* encodedString = [mailtoString stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding];
		//Open default mail client
#if defined(__DAVAENGINE_IPHONE__)
		return [[UIApplication sharedApplication] openURL:[NSURL URLWithString:encodedString]];
#elif defined(__DAVAENGINE_MACOS__)
		return [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:encodedString]];
#endif
	}
#endif
}

