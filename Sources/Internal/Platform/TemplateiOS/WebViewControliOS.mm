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

namespace DAVA
{
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
}

WebViewControl::~WebViewControl()
{
	UIWebView* innerWebView = (UIWebView*)webViewPtr;

	[innerWebView removeFromSuperview];
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
	NSURL* url = [NSURL URLWithString:nsURLPathToOpen];
	
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