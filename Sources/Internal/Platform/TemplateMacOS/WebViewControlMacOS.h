//
//  WebViewControlMacOS.h
//  Framework
//
//  Created by Yuri Coder on 2/15/13.
//
//

#ifndef __DAVAENGINE_WEBVIEWCONTROL_MACOS_H__
#define __DAVAENGINE_WEBVIEWCONTROL_MACOS_H__

//#include "BaseTypes.h"

#include "Math/MathConstants.h"
#include "Math/Math2D.h"
#include "Math/Vector.h"
#include "Math/Rect.h"

namespace DAVA {

// Web View Control - MacOS version.
class WebViewControl
{
public:
	WebViewControl();
	virtual ~WebViewControl();
	
	// Initialize the control.
	void Initialize(const Rect& rect);
	
	// Open the URL requested.
	void OpenURL(const String& urlToOpen);

	// Size/pos/visibility changes.
	void SetRect(const Rect& rect);
	void SetVisible(bool isVisible, bool hierarchic);

protected:
	//A pointer to MacOS WebView.
	void* webViewPtr;
	
	// A pointer to the WebView delegate.
	void* webViewDelegatePtr;
};

};

#endif /* defined(__DAVAENGINE_WEBVIEWCONTROL_MACOS_H__) */
