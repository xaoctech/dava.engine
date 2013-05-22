/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "WebViewControl.h"
#include "FileSystem/Logger.h"

namespace DAVA
{

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

	return delegate->URLChanged(control->webView, newURL, false);
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

}//namespace DAVA
