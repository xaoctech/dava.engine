#ifndef __DAVAENGINE_WEBVIEWCONTROL_H__
#define __DAVAENGINE_WEBVIEWCONTROL_H__

#include "../../UI/IWebViewControl.h"
#include "JniExtensions.h"

namespace DAVA {

class JniWebView;

// Web View Control - Android version.
class WebViewControl : public IWebViewControl
{
	friend class JniWebView;
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

	virtual void SetDelegate(IUIWebViewDelegate *delegate, UIWebView* webView);

private:
	static int webViewIdCount;
	int webViewId;
	IUIWebViewDelegate *delegate;
	UIWebView* webView;
};

class JniWebView: public JniExtension
{
public:
	JniWebView();

	void Initialize(WebViewControl* control, int id, const Rect& rect);
	void Deinitialize(int id);

	void OpenURL(int id, const String& urlToOpen);

	void SetRect(int id, const Rect& rect);
	void SetVisible(int id, bool isVisible);

	IUIWebViewDelegate::eAction URLChanged(int id, const String& newURL);

public:
	static JniWebView* jniWebView;

private:
	typedef std::map<int, WebViewControl*> CONTROLS_MAP;
	CONTROLS_MAP controls;
};

};

#endif /* defined(__DAVAENGINE_WEBVIEWCONTROL_MACOS_H__) */
