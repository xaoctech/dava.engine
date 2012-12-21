/*==================================================================================
 Copyright (c) 2008, DAVA Consulting, LLC
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
 * Neither the name of the DAVA Consulting, LLC nor the
 names of its contributors may be used to endorse or promote products
 derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 
 Revision History:
 * Created by Vitaliy Borodovsky
 =====================================================================================*/

#ifndef __DAVAENGINE_CORE_PLATFORM_ANDROID_H__
#define __DAVAENGINE_CORE_PLATFORM_ANDROID_H__

#include "DAVAEngine.h"
#if defined(__DAVAENGINE_ANDROID__)

#include "AndroidSpecifics.h"

namespace DAVA
{

/**
 * Our saved state data.
 */
struct SavedState
{
    SavedState();
    SavedState(const SavedState& object);
    
    int32 dummy;
//    float angle;
//    int32_t x;
//    int32_t y;
};
    
class ThreadContext;
class EGLRenderer;
class CorePlatformAndroid: public Core
{
    
private:
    AppHandle appHandle;
    
    ASensorManager* sensorManager;
    const ASensor* accelerometerSensor;
    ASensorEventQueue* sensorEventQueue;
    
    AAssetManager * assetManager;

    SavedState savedState;

    bool animating;
    
    bool willQuit;
    bool davaWasInitialized;

    EGLRenderer *renderer;
    
public:

	CorePlatformAndroid();
    virtual ~CorePlatformAndroid();

    bool InitializeAndroidEnvironment(AppHandle handle);
    bool InitializeGLContext();
    bool InitializeWindow();
    bool InitializeEngine();
    
    void Shutdown();
    
    virtual void Run();

	AAssetManager * GetAssetManager();
    
    ThreadContext * CreateThreadContext();
    void BindThreadContext(ThreadContext *context);
    void UnbindThreadContext(ThreadContext *context);
    void ReleaseThreadContext(ThreadContext *context);

    
private:
    
    static int32 HandleInput(AppHandle handle, AInputEvent* event);
    static void HandleCommand(AppHandle handle, int32 cmd);
    
    void DoFrame();
    
    
public:
    
    
//old
    
    
//	virtual void CreateAndroidWindow(const char8 *docPath, const char8 *assets, const char8 *logTag, AndroidSystemDelegate * sysDelegate);
//
//	virtual void Quit();
//
//	void RenderRecreated();
//	void ResizeView(int32 w, int32 h);
//	void RepaintView();
//
//	// called from Activity and manage a visible lifetime
//	void StartVisible();
//	void StopVisible();
//
//	void StartForeground();
//	void StopForeground();
//
//	void OnCreateActivity();
//	void OnDestroyActivity();
//
//	void KeyUp(int32 keyCode);
//	void KeyDown(int32 keyCode);
//
//	void OnTouch(int32 action, int32 id, float32 x, float32 y, long time);
//
//	bool DownloadHttpFile(const String & url, const String & documentsPathname);
//
//	void SetAssetManager(AAssetManager * mngr);
	
private:

//	void QuitAction();
//	void ProcessWithoutDrawing();
//
//	void UpdateScreenMode();


private:
//	DisplayMode windowedMode;
//	int32 oldWidth;
//	int32 oldHeight;
//
//	bool wasCreated;
//	bool renderIsActive;
//
//	bool foreground;
//
//    UIEvent CreateTouchEvent(int32 action, int32 id, float32 x, float32 y, long time);
//    
//	Vector<DAVA::UIEvent> totalTouches;
//	int32 touchPhase;
//
//	AndroidSystemDelegate *androidDelegate;
};
};
#endif // #if defined(__DAVAENGINE_ANDROID__)
#endif // __DAVAENGINE_CORE_PLATFORM_ANDROID_H__
