//
//  WebViewControliOS.cpp
//  Framework
//
//  Created by Yuri Coder on 2/15/13.
//
//

#include "WebViewControliOS.h"
#include "DAVAEngine.h"

#import <UIKit/UIKit.h>
#import <HelperAppDelegate.h>

@interface WebViewURLDelegate : NSObject<UIWebViewDelegate>
{
	DAVA::IUIWebViewDelegate* delegate;
	DAVA::UIWebView* webView;
}

- (id)init;

- (void)setDelegate:(DAVA::IUIWebViewDelegate*)d andWebView:(DAVA::UIWebView*)w;

- (BOOL)webView: (UIWebView*)webView shouldStartLoadWithRequest:(NSURLRequest *)request navigationType:(UIWebViewNavigationType)navigationType;

- (void)webViewDidFinishLoad:(UIWebView *)webView;

@end

@implementation WebViewURLDelegate

- (id)init
{
	self = [super init];
	if (self)
	{
		delegate = NULL;
		webView = NULL;
	}
	return self;
}

- (void)setDelegate:(DAVA::IUIWebViewDelegate *)d andWebView:(DAVA::UIWebView *)w
{
	if (d && w)
	{
		delegate = d;
		webView = w;
	}
}

- (BOOL)webView:(UIWebView *)webView shouldStartLoadWithRequest:(NSURLRequest *)request navigationType:(UIWebViewNavigationType)navigationType
{
	BOOL process = YES;
	
	if (delegate && self->webView)
	{
		NSString* url = [[request URL] absoluteString];
		
		if (url)
		{
			DAVA::IUIWebViewDelegate::eAction action = delegate->URLChanged(self->webView, [url UTF8String], self->webView->IsRedirectedByMouseClick());
			
			switch (action) {
				case DAVA::IUIWebViewDelegate::PROCESS_IN_WEBVIEW:
					DAVA::Logger::Debug("PROCESS_IN_WEBVIEW");
					process = YES;
					break;
					
				case DAVA::IUIWebViewDelegate::PROCESS_IN_SYSTEM_BROWSER:
					DAVA::Logger::Debug("PROCESS_IN_SYSTEM_BROWSER");
					[[UIApplication sharedApplication] openURL:[request URL]];
					process = NO;
					break;
					
				case DAVA::IUIWebViewDelegate::NO_PROCESS:
					DAVA::Logger::Debug("NO_PROCESS");
					
				default:
					process = NO;
					break;
			}
		}
	}
	
	return process;
}


- (void)webViewDidFinishLoad:(UIWebView *)webView
{
    if (delegate && self->webView)
	{
        delegate->PageLoaded(self->webView);
    }
}
@end


namespace DAVA
{
	typedef DAVA::UIWebView DAVAWebView;

	//Use unqualified UIWebView and UIScreen from global namespace, i.e. from UIKit
	using ::UIWebView;
	using ::UIScreen;

WebViewControl::WebViewControl()
{
	CGRect emptyRect = CGRectMake(0.0f, 0.0f, 0.0f, 0.0f);
	webViewPtr = [[UIWebView alloc] initWithFrame:emptyRect];

	UIWebView* localWebView = (UIWebView*)webViewPtr;
	HelperAppDelegate* appDelegate = [[UIApplication sharedApplication] delegate];
	[[[appDelegate glController] view] addSubview:localWebView];

	webViewURLDelegatePtr = [[WebViewURLDelegate alloc] init];
	[localWebView setDelegate:(WebViewURLDelegate*)webViewURLDelegatePtr];

	[localWebView becomeFirstResponder];
}

WebViewControl::~WebViewControl()
{
	UIWebView* innerWebView = (UIWebView*)webViewPtr;

	[innerWebView removeFromSuperview];
	[innerWebView release];
	webViewPtr = nil;

	WebViewURLDelegate* w = (WebViewURLDelegate*)webViewURLDelegatePtr;
	[w release];
	webViewURLDelegatePtr = nil;
}
	
void WebViewControl::SetDelegate(IUIWebViewDelegate *delegate, DAVAWebView* webView)
{
	WebViewURLDelegate* w = (WebViewURLDelegate*)webViewURLDelegatePtr;
	[w setDelegate:delegate andWebView:webView];
}

void WebViewControl::Initialize(const Rect& rect)
{
	SetRect(rect);
}

// Open the URL requested.
void WebViewControl::OpenURL(const String& urlToOpen)
{
	NSString* nsURLPathToOpen = [NSString stringWithUTF8String:urlToOpen.c_str()];
	NSURL* url = [NSURL URLWithString:[nsURLPathToOpen stringByAddingPercentEscapesUsingEncoding: NSUTF8StringEncoding]];
	
	NSURLRequest* requestObj = [NSURLRequest requestWithURL:url];
	[(UIWebView*)webViewPtr loadRequest:requestObj];
}

void WebViewControl::SetRect(const Rect& rect)
{
	CGRect webViewRect = [(UIWebView*)webViewPtr frame];

	Core::eScreenOrientation screenOrientation = Core::Instance()->GetScreenOrientation();
	switch (screenOrientation)
	{
		case Core::SCREEN_ORIENTATION_LANDSCAPE_LEFT:
		{
			// X and Y are swapped in this case.
			webViewRect.origin.y = (DAVA::Core::Instance()->GetVirtualScreenXMax() - rect.x - rect.dx) * DAVA::Core::GetVirtualToPhysicalFactor();
			webViewRect.origin.x = rect.y * DAVA::Core::GetVirtualToPhysicalFactor();
			
			webViewRect.origin.x += Core::Instance()->GetPhysicalDrawOffset().y;
			webViewRect.origin.y += Core::Instance()->GetPhysicalDrawOffset().x;

			// Height and width are swapped in this case,
			webViewRect.size.width = rect.dy * DAVA::Core::GetVirtualToPhysicalFactor();
			webViewRect.size.height = rect.dx * DAVA::Core::GetVirtualToPhysicalFactor();
			
			((UIWebView*)webViewPtr).transform = CGAffineTransformMakeRotation(DAVA::DegToRad(-90.0f));
			break;
		}

		case Core::SCREEN_ORIENTATION_LANDSCAPE_RIGHT:
		{
			// X and Y are swapped in this case.
			webViewRect.origin.y = rect.x * DAVA::Core::GetVirtualToPhysicalFactor();
			webViewRect.origin.x = (DAVA::Core::Instance()->GetVirtualScreenYMax() - rect.y - rect.dy) * DAVA::Core::GetVirtualToPhysicalFactor();
			
			// Height and width are swapped in this case,
			webViewRect.size.width = rect.dy * DAVA::Core::GetVirtualToPhysicalFactor();
			webViewRect.size.height = rect.dx * DAVA::Core::GetVirtualToPhysicalFactor();
			
			((UIWebView*)webViewPtr).transform = CGAffineTransformMakeRotation(DAVA::DegToRad(90.0f));
			break;
		}

		case Core::SCREEN_ORIENTATION_PORTRAIT:
		case Core::SCREEN_ORIENTATION_PORTRAIT_UPSIDE_DOWN:
		{
			// Minimum recalculations are needed, no swapping, no rotation.
			webViewRect.origin.x = rect.x * DAVA::Core::GetVirtualToPhysicalFactor();
			webViewRect.origin.y = rect.y * DAVA::Core::GetVirtualToPhysicalFactor();
			
			webViewRect.size.width = rect.dx * DAVA::Core::GetVirtualToPhysicalFactor();
			webViewRect.size.height = rect.dy * DAVA::Core::GetVirtualToPhysicalFactor();

			webViewRect.origin.x += Core::Instance()->GetPhysicalDrawOffset().x;
			webViewRect.origin.y += Core::Instance()->GetPhysicalDrawOffset().y;

			break;
		}

		default:
		{
			break;
		}
	}
	
	// Apply the Retina scale divider, if any.
	float scaleDivider = GetScaleDivider();
	webViewRect.origin.x /= scaleDivider;
	webViewRect.origin.y /= scaleDivider;
	webViewRect.size.height /= scaleDivider;
	webViewRect.size.width /= scaleDivider;

	[(UIWebView*)webViewPtr setFrame: webViewRect];
}

void WebViewControl::SetVisible(bool isVisible, bool hierarchic)
{
	[(UIWebView*)webViewPtr setHidden:!isVisible];
}

float WebViewControl::GetScaleDivider()
{
    float scaleDivider = 1.f;
    if (DAVA::Core::IsAutodetectContentScaleFactor())
    {
        if ([UIScreen instancesRespondToSelector: @selector(scale) ]
            && [UIView instancesRespondToSelector: @selector(contentScaleFactor) ])
        {
            scaleDivider = [[UIScreen mainScreen] scale];
        }
	}

	return scaleDivider;
}
    
};