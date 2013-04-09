/*
 *  CorePlatformAndroid.h
 *  TemplateProjectAndroid
 *
 *  Created by Victor  Kleschenko on 2/18/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */


#ifndef __DAVAENGINE_CORE_PLATFORM_ANDROID_H__
#define __DAVAENGINE_CORE_PLATFORM_ANDROID_H__

#include "DAVAEngine.h"
#if defined(__DAVAENGINE_ANDROID__)

#include "AndroidSpecifics.h"
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

namespace DAVA
{

class AndroidSystemDelegate
{
public:
	AndroidSystemDelegate(JavaVM *vm);

	virtual void ShowKeyboard() = 0;
	virtual void HideKeyboard() = 0;
	virtual bool DownloadHttpFile(const String & url, const String & documentsPathname) = 0;
    
    virtual GLint RenderBuffer() = 0;
	virtual GLint FrameBuffer() = 0;

	JNIEnv* GetEnvironment() const {return environment;};
	JavaVM* GetVM() const {return vm;};
protected:
	JNIEnv* environment;
	JavaVM* vm;
};



class Thread;
class CorePlatformAndroid: public Core
{
public:

	CorePlatformAndroid();

	virtual void CreateAndroidWindow(const char8 *docPath, const char8 *assets, const char8 *logTag, AndroidSystemDelegate * sysDelegate);

	virtual void Quit();

	void RenderRecreated(int32 w, int32 h);
	void RepaintView();

	// called from Activity and manage a visible lifetime
	void StartVisible();
	void StopVisible();

	void StartForeground();
	void StopForeground();

	void OnCreateActivity();
	void OnDestroyActivity();

	void KeyUp(int32 keyCode);
	void KeyDown(int32 keyCode);

	void OnInput(int32 action, int32 id, float32 x, float32 y, long time, int32 source);

	bool DownloadHttpFile(const String & url, const String & documentsPathname);

	AAssetManager * GetAssetManager();
	void SetAssetManager(AAssetManager * mngr);

	const String& GetExternalStoragePathname() const {return externalStorage;};
	
	AndroidSystemDelegate* GetAndroidSystemDelegate() const;

private:

	void QuitAction();
	void ProcessWithoutDrawing();

	void UpdateScreenMode();

    void ResizeView(int32 w, int32 h);

    

private:
	int32 width;
	int32 height;

	bool wasCreated;
	bool renderIsActive;

	bool foreground;

	UIEvent CreateInputEvent(int32 action, int32 id, float32 x, float32 y, long time, int32 source);

	Vector<DAVA::UIEvent> totalTouches;
	int32 touchPhase;

	AndroidSystemDelegate *androidDelegate;
	AAssetManager * assetMngr;

	String externalStorage;
};
};
#endif // #if defined(__DAVAENGINE_ANDROID__)
#endif // __DAVAENGINE_CORE_PLATFORM_ANDROID_H__
