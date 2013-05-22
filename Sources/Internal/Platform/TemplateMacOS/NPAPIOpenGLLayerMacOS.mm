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

- (id) initWithPluginInstance:(NPAPIPluginMacOS*) instance
{
    if ([super init])
	{
		pluginInstance = instance;
		isFirstDraw = YES;
        m_angle = 0;
    }
	
    return self;
}

- (void)drawInCGLContext:(CGLContextObj)ctx pixelFormat:(CGLPixelFormatObj)pf forLayerTime:(CFTimeInterval)t displayTime:(const CVTimeStamp *)ts
{
	if (isFirstDraw == YES && pluginInstance)
	{
		[pluginInstance doInitializationOnFirstDraw];
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		isFirstDraw = NO;
		
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
	DAVA::Core::Instance()->SystemProcessFrame();
	
//    if(DAVA::Core::Instance()->IsActive())
//    {
//		NSOpenGLContext* context = [[NSOpenGLContext alloc] initWithCGLContextObj:ctx];
//		[context flushBuffer];
//    }

	[super drawInCGLContext:ctx pixelFormat:pf forLayerTime:t displayTime:ts];
	DAVA::RenderManager::Instance()->Unlock();
}

@end
