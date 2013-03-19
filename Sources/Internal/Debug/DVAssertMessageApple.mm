#include "Debug/DVAssertMessage.h"

#if defined(__DAVAENGINE_MACOS__) 
#import <Foundation/Foundation.h>
#include <AppKit/NSAlert.h>
#elif defined(__DAVAENGINE_IPHONE__)
#include "UI/UIScreenManager.h"
#import "UIAlertView_Modal.h"
#endif


void DAVA::DVAssertMessage::InnerShow(const char* content)
{
#if defined(__DAVAENGINE_MACOS__)
    NSString *contents = [NSString stringWithUTF8String:content];
    
    NSAlert *alert = [[NSAlert alloc] init];
    [alert setMessageText:@"Assert"];
    [alert setInformativeText:contents];
    [alert runModal];
    [alert release];
    [contents release];
#elif defined(__DAVAENGINE_IPHONE__)
    NSString *contents = [NSString stringWithUTF8String:content];

    // Yuri Coder, 2013/02/06. This method is specific for iOS-implementation only,
    // it blocks drawing to avoid deadlocks. See EAGLView.mm file for details.
    UIScreenManager::Instance()->BlockDrawing();

    UIAlertView* alert = [[UIAlertView alloc] initWithTitle:@"Assert" message:contents delegate:nil cancelButtonTitle:@"OK" otherButtonTitles: nil];
    
    [alert performSelectorOnMainThread:@selector(showModal) withObject:nil waitUntilDone:YES];
    
#endif

}

