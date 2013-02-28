#include "WebViewControl.h"

using namespace DAVA;

JniWebView* JniWebView::jniWebView = NULL;
int WebViewControl::webViewIdCount = 0;

JniWebView::JniWebView() :
	JniExtension("com/dava/framework/JNIWebView")
{

}

void JniWebView::Initialize(int id, const Rect& controlRect)
{
	Rect rect = V2P(controlRect);
	jmethodID mid = GetMethodID("Initialize", "(IFFFF)V");
	if (mid)
	{
		GetEnvironment()->CallStaticVoidMethod(javaClass, mid, id, rect.x, rect.y, rect.dx, rect.dy);
	}
}

void JniWebView::Deinitialize(int id)
{
	jmethodID mid = GetMethodID("Deinitialize", "(I)V");
	if (mid)
	{
		GetEnvironment()->CallStaticVoidMethod(javaClass, mid, id);
	}
}

void JniWebView::OpenURL(int id, const String& urlToOpen)
{
	jmethodID mid = GetMethodID("OpenURL", "(ILjava/lang/String;)V");
	if (mid)
	{
		jstring jUrlToOpen = GetEnvironment()->NewStringUTF(urlToOpen.c_str());
		GetEnvironment()->CallStaticVoidMethod(javaClass, mid, id, jUrlToOpen);
		GetEnvironment()->DeleteLocalRef(jUrlToOpen);
	}
}

void JniWebView::SetRect(int id, const Rect& controlRect)
{
	Rect rect = V2P(controlRect);
	jmethodID mid = GetMethodID("SetRect", "(IFFFF)V");
	if (mid)
	{
		GetEnvironment()->CallStaticVoidMethod(javaClass, mid, id, rect.x, rect.y, rect.dx, rect.dy);
	}
}

void JniWebView::SetVisible(int id, bool isVisible)
{
	jmethodID mid = GetMethodID("SetVisible", "(IZ)V");
	if (mid)
	{
		GetEnvironment()->CallStaticVoidMethod(javaClass, mid, id, isVisible);
	}
}

WebViewControl::WebViewControl()
{
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
	JniWebView::jniWebView->Initialize(webViewId, rect);
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
