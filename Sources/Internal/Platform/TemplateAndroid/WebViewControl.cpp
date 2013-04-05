#include "WebViewControl.h"
#include "FileSystem/Logger.h"

using namespace DAVA;

JniWebView* JniWebView::jniWebView = NULL;
int WebViewControl::webViewIdCount = 0;

JniWebView::JniWebView() :
	JniExtension("com/dava/framework/JNIWebView")
{

}

void JniWebView::Initialize(WebViewControl* control, int id, const Rect& controlRect)
{
	controls[id] = control;
	Rect rect = V2P(controlRect);

	jclass javaClass = GetJavaClass();
	if (!javaClass)
		return;

	jmethodID mid = GetMethodID(javaClass, "Initialize", "(IFFFF)V");
	if (mid)
		GetEnvironment()->CallStaticVoidMethod(javaClass, mid, id, rect.x, rect.y, rect.dx, rect.dy);
	ReleaseJavaClass(javaClass);
}

void JniWebView::Deinitialize(int id)
{
	jclass javaClass = GetJavaClass();
	if (!javaClass)
		return;

	controls.erase(id);
	jmethodID mid = GetMethodID(javaClass, "Deinitialize", "(I)V");
	if (mid)
		GetEnvironment()->CallStaticVoidMethod(javaClass, mid, id);
	ReleaseJavaClass(javaClass);
}

void JniWebView::OpenURL(int id, const String& urlToOpen)
{
	jclass javaClass = GetJavaClass();
	if (!javaClass)
		return;

	jmethodID mid = GetMethodID(javaClass, "OpenURL", "(ILjava/lang/String;)V");
	if (mid)
	{
		jstring jUrlToOpen = GetEnvironment()->NewStringUTF(urlToOpen.c_str());
		GetEnvironment()->CallStaticVoidMethod(javaClass, mid, id, jUrlToOpen);
		GetEnvironment()->DeleteLocalRef(jUrlToOpen);
	}
	ReleaseJavaClass(javaClass);
}

void JniWebView::SetRect(int id, const Rect& controlRect)
{
	jclass javaClass = GetJavaClass();
	if (!javaClass)
		return;

	Rect rect = V2P(controlRect);
	jmethodID mid = GetMethodID(javaClass, "SetRect", "(IFFFF)V");
	if (mid)
	{
		GetEnvironment()->CallStaticVoidMethod(javaClass, mid, id, rect.x, rect.y, rect.dx, rect.dy);
	}
	ReleaseJavaClass(javaClass);
}

void JniWebView::SetVisible(int id, bool isVisible)
{
	jclass javaClass = GetJavaClass();
	if (!javaClass)
		return;

	jmethodID mid = GetMethodID(javaClass, "SetVisible", "(IZ)V");
	if (mid)
	{
		GetEnvironment()->CallStaticVoidMethod(javaClass, mid, id, isVisible);
	}
	ReleaseJavaClass(javaClass);
}

IUIWebViewDelegate::eAction JniWebView::URLChanged(int id, const String& newURL)
{
	CONTROLS_MAP::iterator iter = controls.find(id);
	if (iter == controls.end())
	{
		Logger::Debug("Error web view id=%d", id);
		return IUIWebViewDelegate::PROCESS_IN_WEBVIEW;
	}

	WebViewControl* control = iter->second;
	IUIWebViewDelegate *delegate = control->delegate;
	if (!delegate)
		return IUIWebViewDelegate::PROCESS_IN_WEBVIEW;

	return delegate->URLChanged(control->webView, newURL);
}

WebViewControl::WebViewControl()
{
	delegate = NULL;
	webView = NULL;

	if (!JniWebView::jniWebView)
		JniWebView::jniWebView = new JniWebView();
	webViewId = webViewIdCount++;
}

WebViewControl::~WebViewControl()
{
	JniWebView::jniWebView->Deinitialize(webViewId);
}

void WebViewControl::Initialize(const Rect& rect)
{
	JniWebView::jniWebView->Initialize(this, webViewId, rect);
}

void WebViewControl::OpenURL(const String& urlToOpen)
{
	JniWebView::jniWebView->OpenURL(webViewId, urlToOpen);
}

void WebViewControl::SetRect(const Rect& rect)
{
	JniWebView::jniWebView->SetRect(webViewId, rect);
}

void WebViewControl::SetVisible(bool isVisible, bool hierarchic)
{
	JniWebView::jniWebView->SetVisible(webViewId, isVisible);
}

void WebViewControl::SetDelegate(DAVA::IUIWebViewDelegate *delegate, DAVA::UIWebView* webView)
{
	this->delegate = delegate;
	this->webView = webView;
}
