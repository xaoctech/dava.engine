#ifndef __DAVAENGINE_WEBVIEWCONTROL_H__
#define __DAVAENGINE_WEBVIEWCONTROL_H__

#include "../../UI/IWebViewControl.h"
#include "JniExtensions.h"
#include "Base/BaseTypes.h"

namespace DAVA {

class JniWebView: public JniExtension
{
public:
	JniWebView();

	void Initialize(int id, const Rect& rect);
	void Deinitialize(int id);

	void OpenURL(int id, const String& urlToOpen);

	void SetRect(int id, const Rect& rect);
	void SetVisible(int id, bool isVisible);

public:
	static JniWebView* jniWebView;
};

// Web View Control - Android version.
class WebViewControl : public IWebViewControl
{
public:
	WebViewControl();
	virtual ~WebViewControl();
	
	// Initialize the control.
	virtual void Initialize(const Rect& rect);
	
	// Open the URL requested.
	virtual void OpenURL(const String& urlToOpen);

	// Size/pos/visibility changes.
	virtual void SetRect(const Rect& rect);
	virtual void SetVisible(bool isVisible, bool hierarchic);

    virtual void SetDelegate(DAVA::IUIWebViewDelegate *delegate, DAVA::UIWebView* webView) {/*TODO: need to write code */} ;

private:
	static int webViewIdCount;
	int webViewId;
};

};

#endif /* defined(__DAVAENGINE_WEBVIEWCONTROL_MACOS_H__) */
