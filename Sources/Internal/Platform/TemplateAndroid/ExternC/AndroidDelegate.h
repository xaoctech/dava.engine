#ifndef __ANDROID_DELEGATE_H__
#define __ANDROID_DELEGATE_H__

#include "Base/BaseTypes.h"
#if defined(__DAVAENGINE_ANDROID__)

#include <jni.h>
#include "Platform/TemplateAndroid/CorePlatformAndroid.h"

class AndroidDelegate: public DAVA::AndroidSystemDelegate
{
	jobject classApplication;
	jobject classActivity;

	enum eConst
	{
		MAX_PATH_SZ = 260
	};

	char activityName[MAX_PATH_SZ];
	char httpDownloaderName[MAX_PATH_SZ];
    
    GLint frameBuffer;
    GLint renderBuffer;

public:

	AndroidDelegate(JavaVM *vm);

	void SetApplication(jobject app, char *packageName);
	void SetActivity(jobject activity);
    void SetBuffers(GLint newFrameBuffer, GLint newRenderBuffer);

    virtual GLint RenderBuffer();
	virtual GLint FrameBuffer();
	virtual void ShowKeyboard();
	virtual void HideKeyboard();
	virtual bool DownloadHttpFile(const DAVA::String & url, const DAVA::String & documentsPathname);
};

#endif //#if defined(__DAVAENGINE_ANDROID__)

#endif //#ifndef __ANDROID_LISTENER_H__
