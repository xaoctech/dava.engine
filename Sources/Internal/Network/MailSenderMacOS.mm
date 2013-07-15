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
#import "MailComposeDelegate.h"
#import "UIAlertView_Modal.h"
#include "UIScreenManageriPhone.h"
#include "EAGLViewController.h"

#elif defined(__DAVAENGINE_MACOS__)
#import <Foundation/Foundation.h>
#import <AppKit/NSWorkspace.h>
#endif

namespace DAVA
{
#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_MACOS__)
	bool MailSender::SendEmail(const WideString &email, const WideString &subject, const WideString &messageText)
	{
		// Don't try to open mail client if empty email string is passed
		if (email.empty() || email == L"")
			return false;
			
		// Convert input values into NSString
		NSString* msgEmail = [[[NSString alloc] initWithBytes: email.data()
												length: email.size() * sizeof(wchar_t)
												encoding:NSUTF32LittleEndianStringEncoding] autorelease];
		NSString* msgSubj = [[[NSString alloc] initWithBytes: subject.data()
												length: subject.size() * sizeof(wchar_t)
												encoding:NSUTF32LittleEndianStringEncoding] autorelease];
		NSString* msgBody = [[[NSString alloc] initWithBytes: messageText.data()
												length: messageText.size() * sizeof(wchar_t)
												encoding:NSUTF32LittleEndianStringEncoding] autorelease];
#if defined(__DAVAENGINE_IPHONE__)
		
		EAGLViewController* rootViewController = (EAGLViewController *)UIScreenManager::Instance()->GetController();
		
		MailComposeDelegate* mailDelegate = [[[MailComposeDelegate alloc] initWithController:rootViewController
																					   email:msgEmail
																					 subject:msgSubj
																					 message:msgBody] autorelease];
		
		[mailDelegate retain];
		
		if([mailDelegate show])
		{
			return true;
		}
		UIAlertView* alert = [[[UIAlertView alloc] initWithTitle:@"Notification"
														message:@"Device is unable to send email in its current state."
													   delegate:nil
											  cancelButtonTitle:@"OK"
											  otherButtonTitles: nil] autorelease];
		[alert show];
		
		return false;
#elif defined(__DAVAENGINE_MACOS__)
		// Build mailto string
		NSString* mailtoString = [NSString stringWithFormat:@"mailto:?to=%@&subject=%@&body=%@",
								  msgEmail, msgSubj, msgBody];
		// Build correct web string without special charachters
		NSString* encodedString = [mailtoString stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding];
		return [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:encodedString]];
#endif
	}
#endif
}

