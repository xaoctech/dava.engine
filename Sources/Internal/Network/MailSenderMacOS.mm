//
//  MailSenderMacOS.cpp
//  
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
	bool MailSender::SendEmail(const WideString &email, const WideString &subject, const WideString &messageText)
	{
		// Don't try to open mail client if emзty email string is passed
		if (email.empty() || email == L"")
			return false;
			
		// Convert input values into NSString
		NSString* msgEmail = [[NSString alloc] initWithBytes: email.data()
												length: email.size() * sizeof(wchar_t)
												encoding:NSUTF32LittleEndianStringEncoding];
		NSString* msgSubj = [[NSString alloc] initWithBytes: subject.data()
												length: subject.size() * sizeof(wchar_t)
												encoding:NSUTF32LittleEndianStringEncoding];
		NSString* msgBody = [[NSString alloc] initWithBytes: messageText.data()
												length: messageText.size() * sizeof(wchar_t)
												encoding:NSUTF32LittleEndianStringEncoding];
		// Build mailto string
		NSString* mailtoString = [NSString stringWithFormat:@"mailto:?to=%@&subject=%@&body=%@",
									msgEmail, msgSubj, msgBody];
		// Build correct web string without special charachters
		NSString* encodedString = [mailtoString stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding];
		// Open default mail client
#if defined(__DAVAENGINE_IPHONE__)
		return [[UIApplication sharedApplication] openURL:[NSURL URLWithString:encodedString]];
#elif defined(__DAVAENGINE_MACOS__)
		return [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:encodedString]];
#endif
	}
#endif
}

