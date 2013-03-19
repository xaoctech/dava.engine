//
//  WebViewControlMacOS.cpp
//  Framework
//
//  Created by Yuri Coder on 2/15/13.
//
//

#include "WebViewControlMacOS.h"
#include "MainWindowController.h"

#import <WebKit/WebKit.h>

using namespace DAVA;

// A delegate is needed to block the context menu. Note - this delegate
// is informal, so no inheritance from WebUIDelegate needed.
@interface WebViewControlUIDelegate : NSObject
{
}

// Intercept the right-mouse-clicks/
- (NSArray *)webView:(WebView *)sender contextMenuItemsForElement:(NSDictionary *)element defaultMenuItems:(NSArray *)defaultMenuItems;

@end

@implementation WebViewControlUIDelegate

- (NSArray *)webView:(WebView *)sender contextMenuItemsForElement:(NSDictionary *)element defaultMenuItems:(NSArray *)defaultMenuItems
{
	// No menu items needed.
	return nil;
}

@end


WebViewControl::WebViewControl()
{
	NSRect emptyRect;// = CGRectMake(0.0f, 0.0f, 0.0f, 0.0f);
	webViewPtr = [[WebView alloc] initWithFrame:emptyRect frameName:nil groupName:nil];

	WebView* localWebView = (WebView*)webViewPtr;
	[localWebView setWantsLayer:YES];
	
	webViewDelegatePtr = [[WebViewControlUIDelegate alloc] init];
	[localWebView setUIDelegate:(WebViewControlUIDelegate*)webViewDelegatePtr];

	NSView* openGLView = (NSView*)Core::Instance()->GetOpenGLView();
	[openGLView addSubview:localWebView];
}

WebViewControl::~WebViewControl()
{
	WebView* innerWebView = (WebView*)webViewPtr;

	[innerWebView setUIDelegate:nil];

	[innerWebView removeFromSuperview];
	[innerWebView close];
	[innerWebView release];
	webViewPtr = nil;
}

void WebViewControl::Initialize(const Rect& rect)
{
	SetRect(rect);
}

// Open the URL requested.
void WebViewControl::OpenURL(const String& urlToOpen)
{
	NSString* nsURLPathToOpen = [NSString stringWithUTF8String:urlToOpen.c_str()];
	[(WebView*)webViewPtr setMainFrameURL:nsURLPathToOpen];
}

void WebViewControl::SetRect(const Rect& rect)
{
	NSRect webViewRect = [(WebView*)webViewPtr frame];

	webViewRect.size.width = rect.dx * Core::GetVirtualToPhysicalFactor();
	webViewRect.size.height = rect.dy * Core::GetVirtualToPhysicalFactor();
	
	webViewRect.origin.x = rect.x * DAVA::Core::GetVirtualToPhysicalFactor();
	webViewRect.origin.y = (Core::Instance()->GetPhysicalScreenHeight() - rect.y - rect.dy) * DAVA::Core::GetVirtualToPhysicalFactor();
	
	webViewRect.origin.x += Core::Instance()->GetPhysicalDrawOffset().x;
	webViewRect.origin.y += Core::Instance()->GetPhysicalDrawOffset().y;
	
	[(WebView*)webViewPtr setFrame: webViewRect];
}

void WebViewControl::SetVisible(bool isVisible, bool hierarchic)
{
	[(WebView*)webViewPtr setHidden:!isVisible];
}

