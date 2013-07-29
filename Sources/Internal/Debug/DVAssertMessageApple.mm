#include "Debug/DVAssertMessage.h"

#if defined(__DAVAENGINE_MACOS__) 
#import <Foundation/Foundation.h>
#include <AppKit/NSAlert.h>
#elif defined(__DAVAENGINE_IPHONE__)
#include "UI/UIScreenManager.h"
#import "UIAlertView_Modal.h"
#import "UIDismissionHandlerAlertView.h"
#endif


void DAVA::DVAssertMessage::InnerShow(eModalType modalType, const char* content)
{
#if defined(__DAVAENGINE_MACOS__)
    NSString *contents = [NSString stringWithUTF8String:content];
    
    NSAlert *alert = [[NSAlert alloc] init];
    [alert setMessageText:@"Assert"];
    [alert setInformativeText:contents];
	
	// Modal Types are ignored on MacOS - there is no way to show non-modal alerts on this platform.
    [alert runModal];
    [alert release];
    [contents release];
#elif defined(__DAVAENGINE_IPHONE__)
	static const uint MAX_WARNING_MESSAGE_LENGTH = 8192;
    NSString *contents = [NSString stringWithUTF8String:content];

	switch (modalType)
	{
		case ALWAYS_MODAL:
		{
			// Yuri Coder, 2013/02/06. This method is specific for iOS-implementation only,
			// it blocks drawing to avoid deadlocks. See EAGLView.mm file for details.
			UIScreenManager::Instance()->BlockDrawing();
			
			// Yuri Coder, 2013/07/19. Always display new Alert View in case of ASSERT.
			UIAlertView* alert = [[UIAlertView alloc] initWithTitle:@"Assert" message:contents delegate:nil cancelButtonTitle:@"OK" otherButtonTitles: nil];
			[alert performSelectorOnMainThread:@selector(showModal) withObject:nil waitUntilDone:YES];
			break;
		}

		default:
		{
			bool needRecreateAlertView = true;
			if (messageBoxPtr)
			{
				// An alert already opened - dismiss it and re-create with the new message.
				UIDismissionHandlerAlertView* alert = (UIDismissionHandlerAlertView*)messageBoxPtr;
				NSString* curAlertMessage = [alert getMessage];
				if (curAlertMessage)
				{
					contents = [curAlertMessage stringByAppendingFormat:@"%@\n", contents];
				}
				
				// Don't allow too long strings.
				if ([contents length] <= MAX_WARNING_MESSAGE_LENGTH)
				{
					[alert dismiss:NO];
					messageBoxPtr = NULL;
				}
				else
				{
					// Leave the current Alert View as-is.
					needRecreateAlertView = false;
					return;
				}
			}

			if (needRecreateAlertView)
			{
				// Create the new alert message and show it.
				UIDismissionHandlerAlertView* alert =
					[[[UIDismissionHandlerAlertView alloc] initWithTitle:@"Assert" message:contents
												   cancelButtonTitle:@"OK" otherButtonTitles: nil] autorelease];
				messageBoxPtr = alert;
				[alert showWithDismissHandler:^(NSInteger selectedIndex, BOOL didCancel)
				 {
					 messageBoxPtr = NULL;
				 }];
			}
		}
	}
    
#endif

}

