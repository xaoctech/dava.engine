//
//  NPAPIOpenGLLayerMacOS.m
//  Framework
//
//  Created by Yuri Coder on 5/22/13.
//
//

#import "NPAPIOpenGLLayerMacOS.h"
#import "NPAPIPluginMacOS.h"

#include "Render/RenderManager.h"

@implementation NPAPIOpenGLLayerMacOS

static CGLContextObj inFrameOpenGLContext = NULL;
static CGLPixelFormatObj inFramePixelFormat = NULL;

- (id) initWithPluginInstance:(NPAPIPluginMacOS*) instance
{
    if ([super init])
	{
		pluginInstance = instance;
		isFirstDraw = YES;

		inFrameOpenGLContext = NULL;
    }

    return self;
}

- (CGLPixelFormatObj)copyCGLPixelFormatForDisplayMask:(uint32_t)mask
{
	uint32_t attributes[] =
	{
		kCGLPFADisplayMask, mask,
		kCGLPFANoRecovery,
		kCGLPFAColorSize, 24,
		kCGLPFADepthSize, 16,
		kCGLPFAStencilSize, 8,
		kCGLPFADoubleBuffer,
		kCGLPFAAccelerated,
		0
	};

	CGLPixelFormatObj pixelFormatObj = NULL;
	GLint numPixelFormats = 0;
	CGLChoosePixelFormat((CGLPixelFormatAttribute*)attributes, &pixelFormatObj, &numPixelFormats);
	if(pixelFormatObj == NULL)
	{
		NSLog(@"Error: Could not choose pixel format!");
	}

	return pixelFormatObj;
}

- (void)releaseCGLPixelFormat:(CGLPixelFormatObj)pixelFormat
{
	CGLDestroyPixelFormat(pixelFormat);
}

- (CGLContextObj)copyCGLContextForPixelFormat:(CGLPixelFormatObj)pixelFormat
{
	CGLContextObj contextObj = NULL;
	CGLCreateContext(pixelFormat, NULL, &contextObj);
	if(contextObj == NULL)
	{
		NSLog(@"Error: Could not create context!");
	}

	// enable vsync
	GLint swapInt = 1;
	CGLSetParameter(contextObj, kCGLCPSwapInterval, &swapInt);
	CGLSetCurrentContext(contextObj);

	return contextObj;
}

- (void)releaseCGLContext:(CGLContextObj)glContext
{
	CGLDestroyContext(glContext);
}

- (void)drawInCGLContext:(CGLContextObj)ctx pixelFormat:(CGLPixelFormatObj)pf forLayerTime:(CFTimeInterval)t displayTime:(const CVTimeStamp *)ts
{
	if (isFirstDraw == YES && pluginInstance)
	{
		[pluginInstance doInitializationOnFirstDraw];
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		isFirstDraw = NO;

		[super drawInCGLContext:ctx pixelFormat:pf forLayerTime:t displayTime:ts];
		return;
	}
/*
	if (activeCursor != RenderManager::Instance()->GetCursor())
	{
		activeCursor = RenderManager::Instance()->GetCursor();
		[[self window] invalidateCursorRectsForView: self];
	}
*/

	DAVA::RenderManager::Instance()->Lock();
	
	inFrameOpenGLContext = ctx;
	inFramePixelFormat = pf;

	DAVA::Core::Instance()->SystemProcessFrame();

	// These parameters are valid inside SystemProcessFrame() only.
	inFrameOpenGLContext = NULL;
	inFramePixelFormat = NULL;

	[super drawInCGLContext:ctx pixelFormat:pf forLayerTime:t displayTime:ts];
	DAVA::RenderManager::Instance()->Unlock();
}

+ (void) getThreadChildContext:(CGLContextObj*) childContext
{
	// This method should be called only from inside SystemProcessFrame.
	DVASSERT(inFrameOpenGLContext);
	DVASSERT(inFramePixelFormat);

	CGLError err = CGLCreateContext(inFramePixelFormat, inFrameOpenGLContext, childContext);
	NSLog(@"getThreadChildContext::CGLCreateContext executed with err code %i", err);
}

@end
