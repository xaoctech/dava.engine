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

#include "Platform/Android/CorePlatformAndroid.h"

#if defined(__DAVAENGINE_ANDROID__)

#include "Platform/Android/AndroidSpecifics.h"
#include "Platform/Android/EGLRenderer.h"

extern void FrameworkDidLaunched();
extern void FrameworkWillTerminate();

namespace DAVA
{

int Core::Run(int argc, char * argv[], AppHandle handle)
{
    // Make sure glue isn't stripped.
    app_dummy();
    
    
    sleep(15); //TODO: for debugger start
    
//    //TODO: log current configuration - DEBUG Feature
//    print_cur_config(handle);
    
    
    CorePlatformAndroid * core = new CorePlatformAndroid();
    if(core)
    {
        core->InitializeAndroidEnvironment(handle);
        core->CreateSingletons();

        core->Run();

        core->ReleaseSingletons();
        core->Shutdown();
        
#ifdef ENABLE_MEMORY_MANAGER
        if (DAVA::MemoryManager::Instance() != 0)
        {
            DAVA::MemoryManager::Instance()->FinalLog();
        }
#endif
    }
    
    return 0;
}
    
Core::eDeviceFamily Core::GetDeviceFamily()
{
    CorePlatformAndroid *core = (CorePlatformAndroid *)Core::Instance();
    if(core)
    {
        return core->GetDeviceFamily();
    }
    
    return DEVICE_UNKNOWN;
}

    
SavedState::SavedState()
    : dummy(0)
{
    
}

SavedState::SavedState(const SavedState& object)
{
    dummy = object.dummy;
}


CorePlatformAndroid::CorePlatformAndroid()
{
    appHandle = NULL;
    sensorManager = NULL;
    accelerometerSensor = NULL;
    sensorEventQueue = NULL;
    assetManager = NULL;
    
    willQuit = false;
    davaWasInitialized = false;
    
    animating = false;
    
    renderer = new EGLRenderer();
}

CorePlatformAndroid::~CorePlatformAndroid()
{
    SafeRelease(renderer);
}

bool CorePlatformAndroid::InitializeAndroidEnvironment(AppHandle handle)
{
    appHandle = handle;
    
    appHandle->userData = this;
    appHandle->onAppCmd = CorePlatformAndroid::HandleCommand;
    appHandle->onInputEvent = CorePlatformAndroid::HandleInput;

    // Prepare to monitor accelerometer
    sensorManager = ASensorManager_getInstance();
    
    //TODO: temporary disabled
    accelerometerSensor = ASensorManager_getDefaultSensor(sensorManager, ASENSOR_TYPE_ACCELEROMETER);
    
    sensorEventQueue = ASensorManager_createEventQueue(sensorManager, appHandle->looper, LOOPER_ID_USER, NULL, NULL);
    
    assetManager = handle->activity->assetManager;
    
    if (appHandle->savedState != NULL)
    {
        // We are starting with a previous saved state; restore from it.
        savedState = *(SavedState*)appHandle->savedState;
    }
    
    return true;
}
    
bool CorePlatformAndroid::InitializeGLContext()
{
    renderer->InitializeGL();
    Thread::InitMainThread();

    return true;
}

bool CorePlatformAndroid::InitializeWindow()
{
    float32 scale = 1.0f;
//    if (DAVA::Core::IsAutodetectContentScaleFactor())
//    {
//        if ([::UIScreen instancesRespondToSelector: @selector(scale) ]
//            && [::UIView instancesRespondToSelector: @selector(contentScaleFactor) ])
//        {
//            scale = (unsigned int)[[::UIScreen mainScreen] scale];
//        }
//    }
    
    float32 width = renderer->GetWidth();
    float32 height = renderer->GetHeight();
    
    DAVA::UIControlSystem::Instance()->SetInputScreenAreaSize(width, height);
    DAVA::Core::Instance()->SetPhysicalScreenSize(width*scale, height*scale);
    
    return true;
}
    
bool CorePlatformAndroid::InitializeEngine()
{
    if(davaWasInitialized)
    {
//        RenderResource::InvalidateAllResources();
    }
    else
    {
        davaWasInitialized = true;
        
        FrameworkDidLaunched();
        
        KeyedArchive * options = Core::Instance()->GetOptions();
        
        RenderManager::Create(Core::RENDERER_OPENGL_ES_2_0);
        RenderManager::Instance()->DetectRenderingCapabilities();

        RenderManager::Instance()->InitFBO(renderer->GetColorRenderbuffer(), renderer->GetDefaultFramebuffer());
        RenderManager::Instance()->Init(Core::Instance()->GetPhysicalScreenWidth(), Core::Instance()->GetPhysicalScreenHeight());
        
        Core::Instance()->SystemAppStarted();
    }
    
    return true;
}
    
void CorePlatformAndroid::Shutdown()
{
    renderer->Shutdown();
}
    
void CorePlatformAndroid::Run()
{
    willQuit = false;
    while (!willQuit)
    {
        // Read all pending events.
        int ident = 0;
        int events = 0;
        struct android_poll_source* source = NULL;
        
        // If not animating, we will block forever waiting for events.
        // If animating, we loop until all events are read, then continue
        // to draw the next frame of animation.
        while ((ident=ALooper_pollAll(animating ? 0 : -1, NULL, &events, (void**)&source)) >= 0)
        {
            // Process this event.
            if (source != NULL)
            {
                source->process(appHandle, source);
            }
            
            // If a sensor has data, process it now.
            if (ident == LOOPER_ID_USER)
            {
                if (accelerometerSensor != NULL)
                {
                    ASensorEvent event;
                    while (ASensorEventQueue_getEvents(sensorEventQueue, &event, 1) > 0)
                    {
//                        Logger::Info("accelerometer: x=%f y=%f z=%f", event.acceleration.x, event.acceleration.y, event.acceleration.z);
                    }
                }
            }
            
            // Check if we are exiting.
            if (appHandle->destroyRequested != 0)
            {
                willQuit = true;
                break;
            }
        }

        if (animating && RenderManager::Instance())
        {
            // Drawing is throttled to the screen update rate, so there
            // is no need to do timing here.
//            sleep(1);

        	timespec sleepTime, sleepTime2;
        	sleepTime.tv_nsec = 1000000;

        	nanosleep(&sleepTime, &sleepTime2);
            DoFrame();
        }
    }
    
    Core::Instance()->SystemAppFinished();
    FrameworkWillTerminate();
}

void CorePlatformAndroid::DoFrame()
{
    RenderManager::Instance()->Lock();
    
    if(Core::Instance()->IsActive())
    {
        renderer->BeginFrame();
    }
    
    Core::SystemProcessFrame();

    if(Core::Instance()->IsActive())
    {
        renderer->EndFrame();
    }
    
    RenderManager::Instance()->Unlock();
}
    
Vector<UIEvent> activeTouches;
    
void FillEventData(UIEvent &uiEvent, int32 phase, AInputEvent* event, int32 index)
{
    uiEvent.phase = phase;
    
    uiEvent.physPoint.x = AMotionEvent_getX(event, index);
    uiEvent.physPoint.y = AMotionEvent_getY(event, index);
    
    uiEvent.point.x = uiEvent.physPoint.x;
    uiEvent.point.y = uiEvent.physPoint.y;
    
    uiEvent.timestamp = AMotionEvent_getEventTime(event);
}
    

void ProcessTouches(int32 phase)
{
	DAVA::UIControlSystem::Instance()->OnInput(phase, activeTouches, Vector<UIEvent>());

	for(Vector<UIEvent>::iterator it = activeTouches.begin(); it != activeTouches.end(); )
	{
		if((*it).phase == UIEvent::PHASE_ENDED || (*it).phase == UIEvent::PHASE_CANCELLED)
		{
			it = activeTouches.erase(it);
		}
		else
		{
			++it;
		}
	}
}

void UpdateSingleTouchPosition(int32 phase, AInputEvent* event, int32 index)
{
    int32 id = AMotionEvent_getPointerId(event, index);
    bool isFound = false;
    for(int32 touchIndex = 0; touchIndex < (int32)activeTouches.size(); ++touchIndex)
    {
        if(id == activeTouches[touchIndex].tid)
        {
            FillEventData(activeTouches[touchIndex], phase, event, index);

            isFound = true;
            break;
        }
    }

    DVASSERT_MSG(isFound, "Touch can be found");
    ProcessTouches(phase);
}
    
    
void UpdateTouchPositions(int32 phase, AInputEvent* event)
{
    int32 numEvents = AMotionEvent_getPointerCount(event);
    for (int32 index = 0; index < numEvents; ++index)
    {
        int32 id = AMotionEvent_getPointerId(event, index);
        
        bool isFound = false;
        for(int32 touchIndex = 0; touchIndex < (int32)activeTouches.size(); ++touchIndex)
        {
            if(id == activeTouches[touchIndex].tid)
            {
                FillEventData(activeTouches[touchIndex], phase, event, index);
                
                isFound = true;
                break;
            }
        }
        
        if(!isFound)
        {
            UIEvent uiEvent;
            FillEventData(uiEvent, phase, event, index);
            uiEvent.tid = id;
            activeTouches.push_back(uiEvent);
        }
    }
    
    ProcessTouches(phase);
}
    
int32 CorePlatformAndroid::HandleInput(AppHandle handle, AInputEvent* event)
{
    CorePlatformAndroid *core = (CorePlatformAndroid *)handle->userData;
    
    int32 eventType = AInputEvent_getType(event);
    if (eventType == AINPUT_EVENT_TYPE_MOTION)
    {
//        Logger::Info("***[ TOUCH START ]***");

        
        int32 actionUnmasked = AMotionEvent_getAction(event);
		int32 action = (actionUnmasked & AMOTION_EVENT_ACTION_MASK);

//        Logger::Info("Action unmasked = %d", actionUnmasked);
//        Logger::Info("Action = %d", action);

        int32 numEvents = AMotionEvent_getPointerCount(event);
//        Logger::Info("Pointers count = %d", numEvents);

        switch (action)
        {
            case AMOTION_EVENT_ACTION_DOWN:
            case AMOTION_EVENT_ACTION_POINTER_DOWN:
                UpdateTouchPositions(UIEvent::PHASE_BEGAN, event);
                break;

            case AMOTION_EVENT_ACTION_CANCEL:
                UpdateTouchPositions(UIEvent::PHASE_CANCELLED, event);
                break;

            case AMOTION_EVENT_ACTION_UP:
                UpdateTouchPositions(UIEvent::PHASE_ENDED, event);
                break;

            case AMOTION_EVENT_ACTION_POINTER_UP:
            {
                int32 actindex = (actionUnmasked & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK);
                actindex >>= AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
                
//                Logger::Debug("[CorePlatformAndroid::HandleInput] pointer up (%d)", actindex);
                UpdateSingleTouchPosition(UIEvent::PHASE_ENDED, event, actindex);
                break;
            }

            case AMOTION_EVENT_ACTION_MOVE:
                UpdateTouchPositions(DAVA::UIEvent::PHASE_DRAG, event);
                break;
                
            default:
                Logger::Warning("[CorePlatformAndroid::HandleInput] Unhandled motion event!");
                break;
        }
        
//        Logger::Info("***[ TOUCH END ]***");

        return 1;
    }
    else if(eventType == AINPUT_EVENT_TYPE_KEY)
    {
//        /* Get the key event action. */
//        int32_t AKeyEvent_getAction(const AInputEvent* key_event);
//        
//        /* Get the key event flags. */
//        int32_t AKeyEvent_getFlags(const AInputEvent* key_event);
//        
//        /* Get the key code of the key event.
//         * This is the physical key that was pressed, not the Unicode character. */
//        int32_t AKeyEvent_getKeyCode(const AInputEvent* key_event);
//        
//        /* Get the hardware key id of this key event.
//         * These values are not reliable and vary from device to device. */
//        int32_t AKeyEvent_getScanCode(const AInputEvent* key_event);
//        
//        /* Get the meta key state. */
//        int32_t AKeyEvent_getMetaState(const AInputEvent* key_event);
//        
//        /* Get the repeat count of the event.
//         * For both key up an key down events, this is the number of times the key has
//         * repeated with the first down starting at 0 and counting up from there.  For
//         * multiple key events, this is the number of down/up pairs that have occurred. */
//        int32_t AKeyEvent_getRepeatCount(const AInputEvent* key_event);
//        
//        /* Get the time of the most recent key down event, in the
//         * java.lang.System.nanoTime() time base.  If this is a down event,
//         * this will be the same as eventTime.
//         * Note that when chording keys, this value is the down time of the most recently
//         * pressed key, which may not be the same physical key of this event. */
//        int64_t AKeyEvent_getDownTime(const AInputEvent* key_event);
//        
//        /* Get the time this event occurred, in the
//         * java.lang.System.nanoTime() time base. */
//        int64_t AKeyEvent_getEventTime(const AInputEvent* key_event);
    }
    return 0;
}

void CorePlatformAndroid::HandleCommand(AppHandle handle, int32 cmd)
{
    CorePlatformAndroid *core = (CorePlatformAndroid *)handle->userData;

    switch (cmd)
    {
        case APP_CMD_START: //::onStart
            Logger::Debug("[HandleCommand] START");
            break;
            
        case APP_CMD_RESUME: //::onResume
        {
            Logger::Debug("[HandleCommand] RESUME");
            
            ApplicationCore * core = Core::Instance()->GetApplicationCore();
            if(core)
            {
                core->OnResume();
            }
            else
            {
                Core::Instance()->SetIsActive(true);
            }

            break;
        }
            
        case APP_CMD_PAUSE: //::onPause
        {
            Logger::Debug("[HandleCommand] PAUSE");

            RenderResource::SaveAllResourcesToSystemMem();
            RenderResource::LostAllResources();

            ApplicationCore * core = Core::Instance()->GetApplicationCore();
            if(core)
            {
                core->OnSuspend();
            }
            else
            {
                Core::Instance()->SetIsActive(false);
            }
            
            break;
        }
            
        case APP_CMD_STOP: //::onStop
            Logger::Debug("[HandleCommand] STOP");
            Core::Instance()->GoBackground(false);

            break;
            
        case APP_CMD_DESTROY: //::onDestory
            Logger::Debug("[HandleCommand] DESTROY");
            break;
            
        case APP_CMD_GAINED_FOCUS: //::onWindowFocusChanged
            Logger::Debug("[HandleCommand] FOCUS GAINED");
            // When our app gains focus, we start monitoring the accelerometer.
            core->animating = 1;
            
            if (core->accelerometerSensor != NULL)
            {
                ASensorEventQueue_enableSensor(core->sensorEventQueue, core->accelerometerSensor);
                // We'd like to get 60 events per second (in us).
                ASensorEventQueue_setEventRate(core->sensorEventQueue, core->accelerometerSensor, (1000L/60)*1000);
            }
            break;
            
        case APP_CMD_LOST_FOCUS: //::onWindowFocusChanged
            Logger::Debug("[HandleCommand] FOCUS LOST");
            // When our app loses focus, we stop monitoring the accelerometer.
            // This is to avoid consuming battery while not being used.
            if (core->accelerometerSensor != NULL)
            {
                ASensorEventQueue_disableSensor(core->sensorEventQueue, core->accelerometerSensor);
            }
            // Also stop animating.
            core->animating = 0;
            //            engine_draw_frame(engine);
            break;
            
        case APP_CMD_INIT_WINDOW: //::onNativeWindowCreated
            Logger::Debug("[HandleCommand] INIT WINDOW");
            core->InitializeGLContext();
            core->renderer->SetWindow(handle->window);
            core->InitializeWindow();
            core->InitializeEngine();
            
            break;
            
        case APP_CMD_WINDOW_RESIZED: //::onNativeWindowResized
            Logger::Debug("[HandleCommand] WIDNOW RESIZED");
//            core->InitializeGLContext();
//            core->renderer->SetWindow(handle->window);
//            core->InitializeWindow();
//            core->InitializeEngine();


            break;
            
        case APP_CMD_TERM_WINDOW: //onNativeWindowDestroyed
            Logger::Debug("[HandleCommand] TERM WINDOW");
            core->renderer->SetWindow(NULL);
            // The window is being hidden or closed, clean it up.
            //            engine_term_display(engine);
            core->willQuit = true;
            break;
            
        case APP_CMD_CONFIG_CHANGED: //::onConfigurationChanged
            Logger::Debug("[HandleCommand] CONFIG CHANGED");
            break;

            
        case APP_CMD_SAVE_STATE:
            Logger::Debug("[HandleCommand] SAVE STATE");
            // The system has asked us to save our current state.  Do so.
            core->appHandle->savedState = new SavedState(core->savedState);
            core->appHandle->savedStateSize = sizeof(SavedState);
            break;
            
            /**
             * Command from main thread: the AInputQueue has changed.  Upon processing
             * this command, android_app->inputQueue will be updated to the new queue
             * (or NULL).
             */
        case APP_CMD_INPUT_CHANGED:
            Logger::Debug("[HandleCommand] INPUT_CHANGED");
            break;
            
            /**
             * Command from main thread: the system needs that the current ANativeWindow
             * be redrawn.  You should redraw the window before handing this to
             * android_app_exec_cmd() in order to avoid transient drawing glitches.
             */
        case APP_CMD_WINDOW_REDRAW_NEEDED:
            Logger::Debug("[HandleCommand] WINDOW_REDRAW_NEEDED");
            break;
            
            /**
             * Command from main thread: the content area of the window has changed,
             * such as from the soft input window being shown or hidden.  You can
             * find the new content rect in android_app::contentRect.
             */
        case APP_CMD_CONTENT_RECT_CHANGED:
            Logger::Debug("[HandleCommand] CONTENT_RECT_CHANGED");
            break;
            
            /**
             * Command from main thread: the system is running low on memory.
             * Try to reduce your memory use.
             */
        case APP_CMD_LOW_MEMORY:
            Logger::Debug("[HandleCommand] LOW_MEMORY");

            core->renderer->SetWindow(NULL);
            core->willQuit = true;
            
            break;
    }
}
    
AAssetManager * CorePlatformAndroid::GetAssetManager()
{
    return assetManager;
}

ThreadContext * CorePlatformAndroid::CreateThreadContext()
{
    return renderer->CreateThreadContext();
}

void CorePlatformAndroid::BindThreadContext(ThreadContext *context)
{
    return renderer->BindThreadContext(context);
}

void CorePlatformAndroid::UnbindThreadContext(ThreadContext *context)
{
    return renderer->UnbindThreadContext(context);
}
    
void CorePlatformAndroid::ReleaseThreadContext(ThreadContext *context)
{
    return renderer->ReleaseThreadContext(context);
}

const char8 * CorePlatformAndroid::GetInternalStoragePathname()
{
    if(appHandle)
    {
        return appHandle->activity->internalDataPath;
    }
    
    return String("").c_str();
}
    
const char8 * CorePlatformAndroid::GetExternalStoragePathname()
{
    if(appHandle)
    {
        return appHandle->activity->externalDataPath;
    }
    
    return String("").c_str();
}
    
Core::eDeviceFamily CorePlatformAndroid::GetDeviceFamily()
{
    if(appHandle)
    {
        int32 screenSize = AConfiguration_getScreenSize(appHandle->config);
        return (screenSize >= ACONFIGURATION_SCREENSIZE_LARGE) ? DEVICE_PAD : DEVICE_HANDSET;
    }
    
    return DEVICE_UNKNOWN;
}

void CorePlatformAndroid::ShowKeyboard()
{
    if(appHandle)
    {
        AConfiguration_setKeyboard(appHandle->config, ACONFIGURATION_KEYBOARD_QWERTY);
        ANativeActivity_showSoftInput(appHandle->activity, ANATIVEACTIVITY_SHOW_SOFT_INPUT_FORCED);
    }

//    /**
//     * Flags for ANativeActivity_showSoftInput; see the Java InputMethodManager
//     * API for documentation.
//     */
//    enum {
//        ANATIVEACTIVITY_SHOW_SOFT_INPUT_IMPLICIT = 0x0001,
//        ANATIVEACTIVITY_SHOW_SOFT_INPUT_FORCED = 0x0002,
//    };
//    
//    /**
//     * Show the IME while in the given activity.  Calls InputMethodManager.showSoftInput()
//     * for the given activity.  Note that this method can be called from
//     * *any* thread; it will send a message to the main thread of the process
//     * where the Java finish call will take place.
//     */
//    void ANativeActivity_showSoftInput(ANativeActivity* activity, uint32_t flags);
}

void CorePlatformAndroid::HideKeyboard()
{
    if(appHandle)
    {
        ANativeActivity_hideSoftInput(appHandle->activity, ANATIVEACTIVITY_SHOW_SOFT_INPUT_FORCED);
    }
}
    

    
//    Core::eDeviceFamily Core::GetDeviceFamily()
//    {
//        //TODO: need to create real function
//        return DEVICE_HANDSET;
//    }
//
//    
//	CoreAndroidPlatform::CoreAndroidPlatform()
//		:	Core()
//	{
////		Logger::Debug("[CoreAndroidPlatform::CoreAndroidPlatform]");
//
//		wasCreated = false;
//		renderIsActive = false;
//		oldWidth = 0;
//		oldHeight = 0;
//
//		foreground = false;
//	}
//
//
//	void CoreAndroidPlatform::Quit()
//	{
//		Logger::Debug("[CoreAndroidPlatform::Quit]");
//		QuitAction();
//	}
//
//	void CoreAndroidPlatform::QuitAction()
//	{
//		Logger::Debug("[CoreAndroidPlatform::QuitAction]");
//
//        if(Core::Instance())
//        {
//            Core::Instance()->SystemAppFinished();
//        }
//
//		FrameworkWillTerminate();
//
//#ifdef ENABLE_MEMORY_MANAGER
//		if (DAVA::MemoryManager::Instance() != 0)
//		{
//			DAVA::MemoryManager::Instance()->FinalLog();
//		}
//#endif
//
//		Logger::Debug("[CoreAndroidPlatform::QuitAction] done");
//	}
//
//
//	void CoreAndroidPlatform::RepaintView()
//	{
//        if(renderIsActive)
//        {
//            DAVA::RenderManager::Instance()->Lock();
//            Core::SystemProcessFrame();
//            DAVA::RenderManager::Instance()->Unlock();
//        }
//	}
//
//	void CoreAndroidPlatform::ResizeView(int32 w, int32 h)
//	{
////		if(oldWidth != w || oldHeight != h)
//		{
//			oldWidth = w;
//			oldHeight = h;
//
//			windowedMode.width = w;
//			windowedMode.height = h;
//
//			UpdateScreenMode();
//		}
//
//	}
//
//	void CoreAndroidPlatform::UpdateScreenMode()
//	{
//		UIControlSystem::Instance()->SetInputScreenAreaSize(windowedMode.width, windowedMode.height);
//		Core::Instance()->SetPhysicalScreenSize(windowedMode.width, windowedMode.height);
//
//        RenderManager::Instance()->InitFBSize(windowedMode.width, windowedMode.height);
////        RenderManager::Instance()->Init(windowedMode.width, windowedMode.height);
//
//        Logger::Debug("[CoreAndroidPlatform::UpdateScreenMode] done");
//    }
//
//	void CoreAndroidPlatform::CreateAndroidWindow(const char8 *docPath, const char8 *assets, const char8 *logTag, AndroidSystemDelegate * sysDelegate)
//	{
//		androidDelegate = sysDelegate;
//
//		Core::CreateSingletons();
//
//        Logger::SetTag(logTag);
////		Logger::Debug("[CoreAndroidPlatform::CreateAndroidWindow] docpath = %s", docPath);
////		Logger::Debug("[CoreAndroidPlatform::CreateAndroidWindow] assets = %s", assets);
//
//		FileSystem::Instance()->SetPath(docPath, assets);
//
//		//////////////////////////////////////////////////////////////////////////
//		windowedMode = DisplayMode(480, 320, 16, 0);
//	}
//    
//    void CoreAndroidPlatform::RenderRecreated()
//	{
//		Logger::Debug("[CoreAndroidPlatform::RenderRecreated] start");
//        
//        renderIsActive = true;
//        
//        Thread::InitMainThread();
//        
//        if(wasCreated)
//		{
//            RenderResource::InvalidateAllResources();   
//        }
//        else
//        {
//            wasCreated = true;
//            
//            Logger::Debug("[CoreAndroidPlatform::] before create renderer");
//            RenderManager::Create(Core::RENDERER_OPENGL_ES_2_0);
//	        RenderManager::Instance()->Init(0, 0);
//
//            RenderManager::Instance()->InitFBO(androidDelegate->RenderBuffer(), androidDelegate->FrameBuffer());
//            Logger::Debug("[CoreAndroidPlatform::] after create renderer");
//            
//            FrameworkDidLaunched();
//            
//            KeyedArchive * options = Core::GetOptions();
//            
//            if (options)
//            {
//                windowedMode.width = options->GetInt32("width");
//                windowedMode.height = options->GetInt32("height");
//                windowedMode.bpp = options->GetInt32("bpp");
//            }
//            
//            Logger::Debug("[CoreAndroidPlatform::] w = %d, h = %d", windowedMode.width, windowedMode.height);
//            
//            RenderManager::Instance()->SetFPS(60);
//            
//            UpdateScreenMode();
//            
//            //////////////////////////////////////////////////////////////////////////
//            Core::Instance()->SystemAppStarted();
//            
//            StartForeground();
//        }
//        
//        Logger::Debug("[CoreAndroidPlatform::RenderRecreated] end");
//	}
//
//    
//    void CoreAndroidPlatform::OnCreateActivity()
//	{
////		Logger::Debug("[CoreAndroidPlatform::OnCreateActivity]");
//	}
//
//
//    void CoreAndroidPlatform::OnDestroyActivity()
//	{
////		Logger::Debug("[CoreAndroidPlatform::OnDestroyActivity]");
//        
//        renderIsActive = false;
//	}
//
//
//	void CoreAndroidPlatform::StartVisible()
//	{
////		Logger::Debug("[CoreAndroidPlatform::StartVisible]");
//	}
//
//	void CoreAndroidPlatform::StopVisible()
//	{
////		Logger::Debug("[CoreAndroidPlatform::StopVisible]");
//	}
//
//	void CoreAndroidPlatform::StartForeground()
//	{
//		Logger::Debug("[CoreAndroidPlatform::StartForeground] start");
//		//TODO: VK: add code for handling
//
//        if(wasCreated)
//        {
//            ApplicationCore * core = Core::Instance()->GetApplicationCore();
//            if(core)
//            {
//                core->OnResume();
//            }
//            
//            foreground = true;
//        }
//		Logger::Debug("[CoreAndroidPlatform::StartForeground] end");
//	}
//
//	void CoreAndroidPlatform::StopForeground()
//	{
//		Logger::Debug("[CoreAndroidPlatform::StopForeground]");
//		//TODO: VK: add code for handling
//
//		RenderResource::SaveAllResourcesToSystemMem();
//		RenderResource::LostAllResources();
//
//        ApplicationCore * core = Core::Instance()->GetApplicationCore();
//        if(core)
//        {
//            core->OnSuspend();
//        }
//
//		foreground = false;
//
//		oldWidth = 0;
//		oldHeight = 0;
//	}
//
//	static Vector<DAVA::UIEvent> activeTouches;
//	void CoreAndroidPlatform::KeyUp(int32 keyCode)
//	{
//		Vector<DAVA::UIEvent> touches;
//		Vector<DAVA::UIEvent> emptyTouches;
//
//		for(Vector<DAVA::UIEvent>::iterator it = activeTouches.begin(); it != activeTouches.end(); it++)
//		{
//			touches.push_back(*it);
//		}
//
//		DAVA::UIEvent ev;
//		ev.keyChar = (char16)keyCode;
//		ev.phase = DAVA::UIEvent::PHASE_KEYCHAR;
//		ev.tapCount = 1;
//		ev.tid = (int32)keyCode;
//
//		touches.push_back(ev);
//
//		UIControlSystem::Instance()->OnInput(0, emptyTouches, touches);
//		touches.pop_back();
//		UIControlSystem::Instance()->OnInput(0, emptyTouches, touches);
//	}
//
//	void CoreAndroidPlatform::KeyDown(int32 keyCode)
//	{
//	}
//
//
//    
//    UIEvent CoreAndroidPlatform::CreateTouchEvent(int32 action, int32 id, float32 x, float32 y, long time)
//    {
//		int32 phase = DAVA::UIEvent::PHASE_DRAG;
//		switch(action)
//		{
//            case 5: //ACTION_POINTER_DOWN
//            case 0: //ACTION_DOWN
//                phase = DAVA::UIEvent::PHASE_BEGAN;
//                break;
//                
//            case 6: //ACTION_POINTER_UP
//            case 1: //ACTION_UP
//                phase = DAVA::UIEvent::PHASE_ENDED;
//                break;
//            case 2: //ACTION_MOVE
//                phase = DAVA::UIEvent::PHASE_DRAG;
//                break;
//                
//            case 3: //ACTION_CANCEL
//                phase = DAVA::UIEvent::PHASE_CANCELLED;
//                break;
//                
//            case 4: //ACTION_OUTSIDE
//                break;
//		}
//        
//        UIEvent newTouch;
//        newTouch.tid = id;
//        newTouch.physPoint.x = x;
//        newTouch.physPoint.y = y;
//        newTouch.phase = phase;
//        newTouch.tapCount = 1;
//        
//        return newTouch;
//    }
//	AAssetManager * CoreAndroidPlatform::GetAssetManager()
//	{
//		return assetMngr;
//	}
//	void CoreAndroidPlatform::SetAssetManager(AAssetManager * mngr)
//	{
//		assetMngr = mngr;
//	}
//	void CoreAndroidPlatform::OnTouch(int32 action, int32 id, float32 x, float32 y, long time)
//	{
////		Logger::Debug("[CoreAndroidPlatform::OnTouch] IN totalTouches.size = %d", totalTouches.size());
////		Logger::Debug("[CoreAndroidPlatform::OnTouch] action is %d, id is %d, x is %f, y is %f, time is %d", action, id, x, y, time);
//        
//        UIEvent touchEvent = CreateTouchEvent(action, id, x, y, time);
//        Vector<DAVA::UIEvent> activeTouches;
//        activeTouches.push_back(touchEvent);
//        
//
//        bool isFound = false;
//        for(Vector<DAVA::UIEvent>::iterator it = totalTouches.begin(); it != totalTouches.end(); ++it)
//		{
//            if((*it).tid == touchEvent.tid)
//            {
//                (*it).physPoint.x = touchEvent.physPoint.x;
//                (*it).physPoint.y = touchEvent.physPoint.y;
//                (*it).phase = touchEvent.phase;
//                (*it).tapCount = touchEvent.tapCount;
//                
//                isFound = true;
//            }
//		}
//        if(!isFound)
//        {
//            totalTouches.push_back(touchEvent);
//        }
//
//		UIControlSystem::Instance()->OnInput(touchEvent.phase, activeTouches, totalTouches);
//
//		for(Vector<DAVA::UIEvent>::iterator it = totalTouches.begin(); it != totalTouches.end(); )
//		{
//            if((DAVA::UIEvent::PHASE_ENDED == (*it).phase) || (DAVA::UIEvent::PHASE_CANCELLED == (*it).phase))
//            {
//                it = totalTouches.erase(it);
//            }
//            else
//            {
//                ++it;
//            }
//		}
////		Logger::Debug("[CoreAndroidPlatform::OnTouch] OUT totalTouches.size = %d", totalTouches.size());
//	}
//
//	bool CoreAndroidPlatform::DownloadHttpFile(const String & url, const String & documentsPathname)
//	{
//		if(androidDelegate)
//		{
//			String path = FileSystem::Instance()->SystemPathForFrameworkPath(documentsPathname);
//			return androidDelegate->DownloadHttpFile(url, path);
//		}
//
//		return false;
//	}
}
#endif // #if defined(__DAVAENGINE_ANDROID__)
