#include "AndroidLayer.h"
#include "WebViewControl.h"

extern "C"
{
	int Java_com_dava_framework_JNIWebView_OnUrlChange(JNIEnv* env, jobject classthis, int id, jstring jUrl)
	{
		int res = 0;
		if (DAVA::JniWebView::jniWebView)
		{
			char url[1024];
			CreateStringFromJni(env, jUrl, url);
			res = DAVA::JniWebView::jniWebView->URLChanged(id, url);
		}
		return res;
	}

	void Java_com_dava_framework_JNIWebView_OnPageLoaded(JNIEnv* env, jobject classthis, int id)
	{
		if (DAVA::JniWebView::jniWebView)
		{
			DAVA::JniWebView::jniWebView->PageLoaded(id);
		}
	}
};
