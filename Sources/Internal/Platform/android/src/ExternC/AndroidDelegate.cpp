#include "AndroidDelegate.h"
#include "FileSystem/Logger.h"

AndroidDelegate::AndroidDelegate(JavaVM *vm) :
	AndroidSystemDelegate(vm)
{
	classActivity = NULL;
	classApplication = NULL;
    
    renderBuffer = 0;
    frameBuffer = 0;
}

void AndroidDelegate::SetApplication(jobject app, char *packageName)
{
	classApplication = app;

	strcpy(activityName, packageName);
	strcat(activityName, ".JNIActivity");

	strcpy(httpDownloaderName, packageName);
	strcat(httpDownloaderName, ".JNIHttpDownloader");
}

void AndroidDelegate::SetActivity(jobject activity)
{
	classActivity = activity;
}

void AndroidDelegate::ShowKeyboard()
{
	if(environment)
	{
		jclass cls = environment->FindClass(activityName);
		jmethodID mid = environment->GetMethodID(cls, "ShowKeyboard", "()V");
		if(mid && classActivity)
		{
			environment->CallVoidMethod(classActivity, mid);
		}
		else
		{
			DAVA::Logger::Error("[AndroidDelegate::ShowKeyboard] Can't find method");
		}

		environment->DeleteLocalRef(cls);
	}
}

void AndroidDelegate::HideKeyboard()
{
	if(environment)
	{
		jclass cls = environment->FindClass(activityName);
		jmethodID mid = environment->GetMethodID(cls, "HideKeyboard", "()V");

		if(mid && classActivity)
		{
			environment->CallVoidMethod(classActivity, mid);
		}
		else
		{
			DAVA::Logger::Error("[AndroidDelegate::HideKeyboard] Can't find method");
		}

		environment->DeleteLocalRef(cls);	
	}
}

bool AndroidDelegate::DownloadHttpFile(const DAVA::String & url, const DAVA::String & documentsPathname)
{
	DAVA::Logger::Debug("[AndroidDelegate::DownloadHttpFile] url=%s", url.c_str());
	DAVA::Logger::Debug("[AndroidDelegate::DownloadHttpFile] docpath=%s", documentsPathname.c_str());

	bool retValue = false;
	if(environment)
	{
		jclass cls = environment->FindClass(httpDownloaderName);
		if(cls)
		{
			jmethodID mid = environment->GetStaticMethodID(cls, "DownloadFileFromUrl", "(Ljava/lang/String;Ljava/lang/String;)Z");

			if(mid)
			{
				jstring jstrUrl = environment->NewStringUTF(url.c_str());
				jstring jstrPath = environment->NewStringUTF(documentsPathname.c_str());

				retValue = environment->CallStaticBooleanMethod(cls, mid, jstrUrl, jstrPath);
			}
			else
			{
				DAVA::Logger::Error("[AndroidDelegate::DownloadHttpFile] Can't find method");
			}

			environment->DeleteLocalRef(cls);	
		}
	}
	return retValue;
}

void AndroidDelegate::SetBuffers(GLint newFrameBuffer, GLint newRenderBuffer)
{
    renderBuffer = newRenderBuffer;
    frameBuffer = newFrameBuffer;
}

GLint AndroidDelegate::RenderBuffer()
{
    return renderBuffer;
}

GLint AndroidDelegate::FrameBuffer()
{
    return frameBuffer;
}

