//
//  NPAPIPluginMacOS.m
//  Framework
//
//  Created by Yuri Coder on 5/22/13.
//
//

#import "NPAPIPluginMacOS.h"

#include "NPAPICorePlatformMacOS.h"
#include "RenderManager.h"

// Arguments passed to the NPAPI plugin instance.
#define NPAPI_PLUGIN_ARGUMENT_BUNDLE_PATH "bundlepath"

extern void FrameworkDidLaunched();
extern void FrameworkWillTerminate();

@implementation NPAPIPluginMacOS

-(NPAPIPluginMacOS*) initWithNPP:(NPP)instance
{
	if (self = [super init])
	{
		self->npp = instance;
		self->npWindow = NULL;
		self->openGLLayer = NULL;
		self->appCore = NULL;
		
		self->bundlePath = NULL;
		
		self->hasFocused = NO;
	}
	
	return self;
}

-(void) dealloc
{
	[self terminateFramework];

	[openGLLayer release];
	openGLLayer = NULL;
	
	[bundlePath release];
	bundlePath = NULL;
	
	[super dealloc];
}

-(NPError) npNew:(NPMIMEType)pluginType npMode:(uint16_t) mode npArgCount:(int16_t)argCount
	  npArgNames:(char*[])argNames npArgValues:(char*[]) argValues npSavedData:(NPSavedData*) savedData
{
	// Lookup for the parameters needed for the initialization.
	NSLog(@"npNew: Params count: %i", argCount);
	
	// Lookup for the Bundle Path - it is needed to properly initialize Resources management.
	bool bundlePathFound = false;
	NSString* bundlePathArgName = [NSString stringWithCString:NPAPI_PLUGIN_ARGUMENT_BUNDLE_PATH encoding:NSASCIIStringEncoding];
	for (int16_t i = 0; i < argCount; i ++)
	{
		NSLog(@"npNew: Param #%i name is %s, value is %s", i, argNames[i], argValues[i]);
		NSString* argName = [NSString stringWithCString:argNames[i] encoding:NSASCIIStringEncoding];
		if ([argName caseInsensitiveCompare:bundlePathArgName] == NSOrderedSame)
		{
			self->bundlePath = [[NSString stringWithCString:argValues[i] encoding:NSASCIIStringEncoding] retain];
			bundlePathFound = true;
		}
	}
	
	if (!bundlePathFound)
	{
		NSLog(@"npNew: The %s parameter for NPAPI DAVA Framework plugin is not specified", NPAPI_PLUGIN_ARGUMENT_BUNDLE_PATH);
		return NPERR_INVALID_PARAM;
	}
	
	// Yuri Coder, 2013/05/22. Do we need to pass these parameters to some other location?
	return NPERR_NO_ERROR;
}

// Set the NPWindow.
-(NPError) npSetWindow:(NPWindow*) window
{
	self->npWindow = window;
	return NPERR_NO_ERROR;
}

// Handle the Cocoa event came from browser.
-(NPError) npHandleEvent:(NPCocoaEvent*) event
{
	switch (event->type)
	{
		case NPCocoaEventWindowFocusChanged:
		{
			return [self handleWindowFocusChanged: event->data.focus.hasFocus];
		}

		// TODO! implement key/mouse tracking here!
		default:
		{
			[self parseEvent:event];
			return NPERR_NO_ERROR;
		}
	}
}

-(void) parseEvent:(NPCocoaEvent*)event
{
	switch (event->type)
	{
		case NPCocoaEventDrawRect:
			break;

		case NPCocoaEventMouseDown:
			[self processEvent:DAVA::UIEvent::PHASE_BEGAN touch:event];
			break;

		case NPCocoaEventMouseUp:
			[self processEvent:DAVA::UIEvent::PHASE_ENDED touch:event];
			break;

		case NPCocoaEventMouseMoved:
			[self processEvent:DAVA::UIEvent::PHASE_MOVE touch:event];
			break;

		case NPCocoaEventMouseEntered:
			break;

		case NPCocoaEventMouseExited:
			break;

		case NPCocoaEventMouseDragged:
			[self processEvent:DAVA::UIEvent::PHASE_DRAG touch:event];
			break;

		case NPCocoaEventKeyDown:
			break;

		case NPCocoaEventKeyUp:
			break;

		case NPCocoaEventFlagsChanged:
			break;

		case NPCocoaEventScrollWheel:
			break;

		case NPCocoaEventTextInput:
			break;

		default:
			break;
	}
}

-(void) moveTouchesToVector:(NPCocoaEvent*)curEvent touchPhase:(int)touchPhase outTouches:(DAVA::Vector<DAVA::UIEvent>*)outTouches
{
	int button = 0;
	button = curEvent->data.mouse.buttonNumber + 1;
	time_t timestamp = time(NULL);

	if (touchPhase == DAVA::UIEvent::PHASE_DRAG)
	{
		for(DAVA::Vector<DAVA::UIEvent>::iterator it = activeTouches.begin(); it != activeTouches.end(); it++)
		{
			NSPoint p;
			p.x = curEvent->data.mouse.pluginX;
			p.y = curEvent->data.mouse.pluginY;

			it->physPoint.x = p.x;
			it->physPoint.y = p.y;

			it->tapCount = DAVA::Max(curEvent->data.mouse.clickCount, 1);
			it->timestamp = timestamp;
			it->phase = touchPhase;
		}
	}

	bool isFind = false;
	for(DAVA::Vector<DAVA::UIEvent>::iterator it = activeTouches.begin(); it != activeTouches.end(); it++)
	{
		if(it->tid == button)
		{
			isFind = true;

			NSPoint p;
			p.x = curEvent->data.mouse.pluginX;
			p.y = curEvent->data.mouse.pluginY;

			it->physPoint.x = p.x;
			it->physPoint.y = p.y;

			it->tapCount = curEvent->data.mouse.clickCount;
			it->timestamp = timestamp;
			it->phase = touchPhase;

			break;
		}
	}

	if(!isFind)
	{
		DAVA::UIEvent newTouch;
		newTouch.tid = button;
		NSPoint p;
		p.x = curEvent->data.mouse.pluginX;
		p.y = curEvent->data.mouse.pluginY;

		newTouch.physPoint.x = p.x;
		newTouch.physPoint.y = p.y;

		newTouch.tapCount = curEvent->data.mouse.clickCount;
		newTouch.timestamp = timestamp;
		newTouch.phase = touchPhase;
		activeTouches.push_back(newTouch);
	}

	for(DAVA::Vector<DAVA::UIEvent>::iterator it = activeTouches.begin(); it != activeTouches.end(); it++)
	{
		outTouches->push_back(*it);
	}

	if(touchPhase == DAVA::UIEvent::PHASE_ENDED || touchPhase == DAVA::UIEvent::PHASE_MOVE)
	{
		for(DAVA::Vector<DAVA::UIEvent>::iterator it = activeTouches.begin(); it != activeTouches.end(); it++)
		{
			if(it->tid == button)
			{
				activeTouches.erase(it);
				break;
			}
		}
	}
}

-(void) processEvent:(int)touchPhase touch:(NPCocoaEvent*)touch
{
	DAVA::Vector<DAVA::UIEvent> touches;
	DAVA::Vector<DAVA::UIEvent> emptyTouches;
	[self moveTouchesToVector:touch touchPhase:touchPhase outTouches:&touches];

	DAVA::UIControlSystem::Instance()->OnInput(touchPhase, emptyTouches, touches);
	touches.clear();
}


// Called when the browser requests us to return a value.
-(NPError) npGetValue:(NPPVariable) variable withParam:(void*)param
{
	switch (variable)
	{
		case NPPVpluginCoreAnimationLayer:
		{
			[self createOpenGLLayer];

			*((CALayer **)param) = openGLLayer;
			return NPERR_NO_ERROR;
		}
			
		default:
		{
			return NPERR_GENERIC_ERROR;
		}
	}
}

-(void) setBundlePath:(NSString*) value
{
	NSLog(@"Bundle Path is set to %@", value);
	self->bundlePath = [value retain];
}

-(void) createOpenGLLayer
{
	if (self->openGLLayer)
	{
		// Already created.
		return;
	}

	NSLog(@"Creating NPAPI OpenGL Layer");
	openGLLayer = [[NPAPIOpenGLLayerMacOS alloc] initWithPluginInstance:self];
	openGLLayer.autoresizingMask = kCALayerWidthSizable | kCALayerHeightSizable;
	openGLLayer.opaque = YES;
	openGLLayer.needsDisplayOnBoundsChange = YES;

	// Yuri Coder, 2013/05/22. Verify whether async mode is OK.
	openGLLayer.asynchronous = YES;
	[openGLLayer retain];   // should be balanced by a -release in NPP_Destroy
	
	// Ready to initialize the framework.
	[self initializeFramework];
}

-(void) initializeFramework
{
	//sleep(15);
	DAVA::NPAPICoreMacOSPlatform * core = new DAVA::NPAPICoreMacOSPlatform();
	core->CreateSingletons();
	
	// A Bundle Path have to be updated for the NPAPI Plugin.
	// Yuri Coder, 2013/05/22. TODO: looks like temporary patch.
	DAVA::FilePath::InitializeBundleNameNPAPI([bundlePath UTF8String]);

	FrameworkDidLaunched();
    DAVA::RenderManager::Create(DAVA::Core::RENDERER_OPENGL);

	appCore = DAVA::Core::GetApplicationCore();
	// TODO! Why animation is needed???
}

-(void) doInitializationOnFirstDraw
{
    DAVA::RenderManager::Instance()->DetectRenderingCapabilities();

	NSRect rect = [openGLLayer frame];
	DAVA::RenderManager::Instance()->Init(rect.size.width, rect.size.height);
	DAVA::UIControlSystem::Instance()->SetInputScreenAreaSize(rect.size.width, rect.size.height);
	DAVA::Core::Instance()->SetPhysicalScreenSize(rect.size.width, rect.size.height);
	
	NSLog(@"[NPAPICoreMacOSPlatform] SystemAppStarted");
	DAVA::Core::Instance()->SystemAppStarted();
}

-(void) terminateFramework
{
	FrameworkWillTerminate();	
}

#pragma mark Event Handlers

-(NPError) handleWindowFocusChanged:(BOOL)newValue
{
	NSLog(@"NPAPI Plugin Focus Changed, new value is %i", newValue);
	
	if (newValue == hasFocused)
	{
		// No change actually happened.
		return NPERR_NO_ERROR;
	}

	hasFocused = newValue;
	if (newValue)
	{
		[self onResume];
	}
	else
	{
		[self onSuspend];
	}

	return NPERR_NO_ERROR;
}

- (void) onSuspend
{
    if(appCore)
    {
        appCore->OnSuspend();
    }
    else
    {
        DAVA::Core::Instance()->SetIsActive(false);
    }
}

- (void) onResume
{
    if(appCore)
    {
        appCore->OnResume();
    }
    else
    {
        DAVA::Core::Instance()->SetIsActive(true);
    }
}

@end
