/* ***** BEGIN LICENSE BLOCK *****
 *
 * THIS FILE IS PART OF THE MOZILLA NPAPI SDK BASIC PLUGIN SAMPLE
 * SOURCE CODE. USE, DISTRIBUTION AND REPRODUCTION OF THIS SOURCE
 * IS GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS 
 * SOURCE IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.
 *
 * THE MOZILLA NPAPI SDK BASIC PLUGIN SAMPLE SOURCE CODE IS
 * (C) COPYRIGHT 2008 by the Mozilla Corporation
 * http://www.mozilla.com/
 *
 * Contributors:
 *  Josh Aas <josh@mozilla.com>
 *
 * ***** END LICENSE BLOCK ***** */

/*
 * This sample plugin uses the Cocoa event model and the Core Graphics
 * drawing model.
 */

#include "NPAPIEntryPoint.h"
#import "OpenGLView.h"

/* Structure containing pointers to functions implemented by the browser. */
static NPNetscapeFuncs* browser;

/* Data for each instance of this plugin. */
typedef struct PluginInstance
{
	NPP npp;
	NPWindow window;
}
PluginInstance;

// TEST CODE!!! SHOULD NOT BE THERE!!!
// TEST CODE!!! SHOULD NOT BE THERE!!!
// TEST CODE!!! SHOULD NOT BE THERE!!!
@interface MyCAOpenGLLayer : CAOpenGLLayer
{
    GLfloat m_angle;
	PluginInstance* m_instance;
}

@end

@implementation MyCAOpenGLLayer

- (id) init:(PluginInstance*) instance
{
    if ([super init])
	{
        m_angle = 0;
		m_instance = instance;
    }

    return self;
}

- (void)drawInCGLContext:(CGLContextObj)ctx pixelFormat:(CGLPixelFormatObj)pf forLayerTime:(CFTimeInterval)t displayTime:(const CVTimeStamp *)ts
{
    m_angle += 1;
    GLsizei width = CGRectGetWidth([self bounds]), height = CGRectGetHeight([self bounds]);
    GLfloat halfWidth = width / 2, halfHeight = height / 2;
	
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glScalef(1.0f, -1.0f, 1.0f);
    glOrtho(0, width, 0, height, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
	
    glRotatef(m_angle, 0.0, 0.0, 1.0);
	
    // Clear the stage.
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Draw UL Quadrant 25% Red
    glBegin(GL_QUADS);
	glColor4f(1.0, 0.0, 0.0, 0.25);
	glVertex3f(0, 0, -1.0f);
	glVertex3f(halfWidth, 0, -1.0f);
	glVertex3f(halfWidth, halfHeight, -1.0f);
	glVertex3f(0, halfHeight, -1.0f);
    glEnd();
    
    // Draw UR Quadrant 50% Green
    glBegin(GL_QUADS);
	glColor4f(0.0, 1.0, 0.0, 0.5);
	glVertex3f(halfWidth, 0, -1.0f);
	glVertex3f(width, 0, -1.0f);
	glVertex3f(width, halfHeight, -1.0f);
	glVertex3f(halfWidth, halfHeight, -1.0f);
    glEnd();
    
    // Draw LR Quadrant 75% Blue
    glBegin(GL_QUADS);
	glColor4f(0.0, 0.0, 1.0, 0.75);
	glVertex3f(halfWidth, halfHeight, -1.0f);
	glVertex3f(width, halfHeight, -1.0f);
	glVertex3f(width, height, -1.0f);
	glVertex3f(halfWidth, height, -1.0f);
    glEnd();
    
    // Draw Center Quadrant 100% Magenta
    glBegin(GL_QUADS);
	glColor4f(1.0, 0.0, 1.0, 1.0);
	glVertex3f(width / 4, height / 4, -1.0f);
	glVertex3f(3 * width / 4, height / 4, -1.0f);
	glVertex3f(3 * width / 4, 3 * height / 4, -1.0f);
	glVertex3f(width / 4,   3 * height / 4, -1.0f);
    glEnd();
    
    [super drawInCGLContext:ctx pixelFormat:pf forLayerTime:t displayTime:ts];
}

-(void) onTick:(NSTimer *)timer
{
	if (browser)
	{
		browser->invalidaterect(m_instance->npp, &m_instance->window.clipRect);
		browser->forceredraw(m_instance->npp);
	}
}


@end
// TEST CODE!!! SHOULD NOT BE THERE!!!
// TEST CODE!!! SHOULD NOT BE THERE!!!
// TEST CODE!!! SHOULD NOT BE THERE!!!
// TEST CODE!!! SHOULD NOT BE THERE!!!

static MyCAOpenGLLayer* openGLLayer = NULL;

extern void FrameworkDidLaunched();
extern void FrameworkWillTerminate();

/* Symbol called once by the browser to initialize the plugin. */
extern "C" NPError NP_Initialize(NPNetscapeFuncs* browserFuncs)
{
	NSLog(@"!!!!!!!!!!!!!!!!NP_INITIALIZE is called!!!!!!!!!!!!");
	// Save the browser function table.
	browser = browserFuncs;

	return NPERR_NO_ERROR;
}

/* Function called by the browser to get the plugin's function table. */
extern "C" NPError NP_GetEntryPoints(NPPluginFuncs* pluginFuncs)
{
  /* Check the size of the provided structure based on the offset of the
     last member we need. */
  if (pluginFuncs->size < (offsetof(NPPluginFuncs, setvalue) + sizeof(void*)))
  {
	  return NPERR_INVALID_FUNCTABLE_ERROR;
  }

  pluginFuncs->newp = NPP_New;
  pluginFuncs->destroy = NPP_Destroy;
  pluginFuncs->setwindow = NPP_SetWindow;
  pluginFuncs->newstream = NPP_NewStream;
  pluginFuncs->destroystream = NPP_DestroyStream;
  pluginFuncs->asfile = NPP_StreamAsFile;
  pluginFuncs->writeready = NPP_WriteReady;
  pluginFuncs->write = (NPP_WriteProcPtr)NPP_Write;
  pluginFuncs->print = NPP_Print;
  pluginFuncs->event = NPP_HandleEvent;
  pluginFuncs->urlnotify = NPP_URLNotify;
  pluginFuncs->getvalue = NPP_GetValue;
  pluginFuncs->setvalue = NPP_SetValue;

  return NPERR_NO_ERROR;
}

/* Function called once by the browser to shut down the plugin. */
extern "C" void NP_Shutdown(void)
{
}

/* Called to create a new instance of the plugin. */
extern "C" NPError NPP_New(NPMIMEType pluginType, NPP instance, uint16_t mode, int16_t argc, char* argn[], char* argv[], NPSavedData* saved)
{
	PluginInstance *newInstance = (PluginInstance*)malloc(sizeof(PluginInstance));
	bzero(newInstance, sizeof(PluginInstance));

	newInstance->npp = instance;
	instance->pdata = newInstance;

	// Select the Core Animation model, since we need accelerated OpenGL.
	NPBool supportsCoreAnimation = false;
	if (browser->getvalue(instance, NPNVsupportsCoreAnimationBool, &supportsCoreAnimation) == NPERR_NO_ERROR && supportsCoreAnimation)
	{
		browser->setvalue(instance, NPPVpluginDrawingModel, (void*)NPDrawingModelCoreAnimation);
	}
	else
	{
		printf("CoreGraphics drawing model not supported, can't create a plugin instance.\n");
		return NPERR_INCOMPATIBLE_VERSION_ERROR;
	}

	// Select the Cocoa event model.
	NPBool supportsCocoaEvents = false;
	if (browser->getvalue(instance, NPNVsupportsCocoaBool, &supportsCocoaEvents) == NPERR_NO_ERROR && supportsCocoaEvents)
	{
		browser->setvalue(instance, NPPVpluginEventModel, (void*)NPEventModelCocoa);
	}
	else
	{
		printf("Cocoa event model not supported, can't create a plugin instance.\n");
		return NPERR_INCOMPATIBLE_VERSION_ERROR;
	}

	return NPERR_NO_ERROR;
}

/* Called to destroy an instance of the plugin. */
extern "C" NPError NPP_Destroy(NPP instance, NPSavedData** save)
{
	free(instance->pdata);
	[openGLLayer release];

	return NPERR_NO_ERROR;
}

/* Called to update a plugin instances's NPWindow. */
extern "C" NPError NPP_SetWindow(NPP instance, NPWindow* window)
{
	NSLog(@"!!!!!!!!!!!!!!!SETWINDOW IS CALLED!!!!!!!!!!!!!!");
	PluginInstance* currentInstance = (PluginInstance*)(instance->pdata);

	currentInstance->window = *window;

	return NPERR_NO_ERROR;
}

extern "C" NPError NPP_NewStream(NPP instance, NPMIMEType type, NPStream* stream, NPBool seekable, uint16_t* stype)
{
  *stype = NP_ASFILEONLY;
  return NPERR_NO_ERROR;
}

extern "C" NPError NPP_DestroyStream(NPP instance, NPStream* stream, NPReason reason)
{
  return NPERR_NO_ERROR;
}

extern "C" int32_t NPP_WriteReady(NPP instance, NPStream* stream)
{
  return 0;
}

extern "C" int32_t NPP_Write(NPP instance, NPStream* stream, int32_t offset, int32_t len, void* buffer)
{
  return 0;
}

extern "C" void NPP_StreamAsFile(NPP instance, NPStream* stream, const char* fname)
{
}

extern "C" void NPP_Print(NPP instance, NPPrint* platformPrint)
{
  
}

extern "C" int16_t NPP_HandleEvent(NPP instance, void* event)
{
	NPCocoaEvent* cocoaEvent = (NPCocoaEvent*)event;
	if (cocoaEvent)
	{
		NSLog(@"!!!!!!!!!!!!!!!NPP_HandleEvent IS CALLED, event type %i!!!!!!!!!!!!!!", cocoaEvent->type);
	}

	return 0;
}

extern "C" void NPP_URLNotify(NPP instance, const char* url, NPReason reason, void* notifyData)
{

}

extern "C" NPError NPP_GetValue(NPP instance, NPPVariable variable, void *value)
{
	PluginInstance* currentInstance = (PluginInstance*)(instance->pdata);
	
    switch (variable)
	{
        case NPPVpluginCoreAnimationLayer:
		{
			if (!openGLLayer)
			{
				NSLog(@"First time OpenGLLayer creation!");
				sleep(10);
				NSLog(@"First time OpenGLLayer creation - done!!!");
				openGLLayer = [[MyCAOpenGLLayer alloc] init:currentInstance];
				openGLLayer.autoresizingMask = kCALayerWidthSizable | kCALayerHeightSizable;
				openGLLayer.needsDisplayOnBoundsChange = YES;

				openGLLayer.asynchronous = YES; // check in Async mode firstly
				[openGLLayer retain];   // should be balanced by a -release in NPP_Destroy
				
//				[NSTimer scheduledTimerWithTimeInterval: 0.5
//															  target: openGLLayer
//															selector:@selector(onTick:)
//															userInfo: nil repeats:YES];
			}
	
			*((CALayer **)value) = openGLLayer;
			return NPERR_NO_ERROR;
		}
			
            
        default:
		{
            return NPERR_GENERIC_ERROR;
		}
	}
}

extern "C" NPError NPP_SetValue(NPP instance, NPNVariable variable, void *value)
{
  return NPERR_GENERIC_ERROR;
}

void createWindows()
{
	NSLog(@"Create Windows is called");

	// launch framework and setup all preferences
    //TODO: maybe we need reorder calls
	FrameworkDidLaunched();
    RenderManager::Create(Core::RENDERER_OPENGL);
	
	//Core::Instance()->Creat();
    
	// do all ground work & setup window itself according to value specified by user
	KeyedArchive * options = DAVA::Core::Instance()->GetOptions();
	int32 width = options->GetInt32("width", 800);
	int32 height = options->GetInt32("height", 600);

	/*
	String title = options->GetString("title", "[set application title using core options property 'title']");
	
	mainWindow = [[NSWindow alloc] initWithContentRect:NSMakeRect((fullscreenMode.width - width) / 2,
																  (fullscreenMode.height - height) / 2, width, height)
											 styleMask:NSTitledWindowMask+NSMiniaturizableWindowMask+NSClosableWindowMask
											   backing:NSBackingStoreBuffered defer:FALSE];
	[mainWindow setDelegate:self];
	 */
	
	NSRect rect;
	rect.origin.x = 0;
	rect.origin.y = 0;
	rect.size.width = width;
	rect.size.height = height;
	
	OpenGLView* openGLView = [[OpenGLView alloc]initWithFrame: NSMakeRect(0, 0, width, height)];
	[openGLView setFrame: rect];
	
	//core = Core::GetApplicationCore();
    RenderManager::Instance()->DetectRenderingCapabilities();
	
	// start animation
	//isAnimating = NO;
	
	//currFPS = RenderManager::Instance()->GetFPS();
    //[self startAnimation];
	
	// make window main
	//[mainWindow makeKeyAndOrderFront:nil];
	//[mainWindow setLevel: NSMainMenuWindowLevel + 1];
	//[mainWindow setTitle:[NSString stringWithFormat:@"%s", title.c_str()]];
	//[mainWindow setAcceptsMouseMovedEvents:YES];
	
	//	if ([mainWindow isMainWindow])
	//	{
	//		NSLog(@"****** Our window is MAIN window");
	//	}
}
