#include "Debug/DVAssertMessage.h"

using namespace DAVA;

#if defined(__DAVAENGINE_MACOS__) 
#import <Foundation/Foundation.h>
#include <AppKit/NSAlert.h>
#elif defined(__DAVAENGINE_IPHONE__)
#include "UIAlertView.h"
#import "UIAlertView_Modal.h"
#include "UI/UIScreenManager.h"
#endif

#if defined(__DAVAENGINE_WIN32__)
WideString s2ws(String& s)
{
    int len;
    int slength = (int)s.length() + 1;
    len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0); 
    wchar_t* buf = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
    WideString r(buf);
    delete[] buf;
    return r;

}
#endif



#if defined (ENABLE_ASSERT_MESSAGE)
void DVAssertMessage::ShowMessage(const char8 * text, ...)
{
	va_list vl;
	va_start(vl, text);
#if defined(__DAVAENGINE_WIN32__)
    
	char tmp[4096] = {0};
	// sizeof(tmp) - 2  - We need two characters for appending "\n" if the number of characters exceeds the size of buffer. 
	_vsnprintf(tmp, sizeof(tmp)-2, text, vl);
	strcat(tmp, "\n");
	String str(tmp);
	MessageBox(HWND_DESKTOP,s2ws(str).c_str(),L"Assert",MB_OK | MB_ICONEXCLAMATION);

#elif defined(__DAVAENGINE_MACOS__)
    NSString * formatString = [NSString stringWithUTF8String:text];
    NSString *contents = [[NSString alloc] initWithFormat:formatString arguments:vl];
    
    NSAlert *alert = [[NSAlert alloc] init];
    [alert setMessageText:@"Assert"];
    [alert setInformativeText:contents];
    [alert runModal];
    [alert release];
    [formatString release];
    [contents release];
#elif defined(__DAVAENGINE_IPHONE__)
    NSString * formatString = [NSString stringWithUTF8String:text];
    NSString *contents = [[NSString alloc] initWithFormat:formatString arguments:vl];

	// Yuri Coder, 2013/02/06. This method is specific for iOS-implementation only,
	// it blocks drawing to avoid deadlocks. See EAGLView.mm file for details.
   	UIScreenManager::Instance()->BlockDrawing();

    UIAlertView* alert = [[UIAlertView alloc] initWithTitle:@"Assert" message:contents delegate:nil cancelButtonTitle:@"OK" otherButtonTitles: nil];
    
    [alert performSelectorOnMainThread:@selector(showModal) withObject:nil waitUntilDone:YES];
    
#endif
    va_end(vl);
}

#else

void DVAssertMessage::ShowMessage(const char8 * /*text*/, ...)
{
	// Do nothing here.
}

#endif	// ENABLE_ASSERT_MESSAGE






