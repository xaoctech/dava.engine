//
//  NPAPIOpenGLLayerMacOS.h
//  Framework
//
//  Created by Yuri Coder on 5/22/13.
//
//

#import <QuartzCore/QuartzCore.h>

@class NPAPIPluginMacOS;
@interface NPAPIOpenGLLayerMacOS : CAOpenGLLayer
{
	NPAPIPluginMacOS* pluginInstance;
	
	// We need to do extra initialization on first draw.
	BOOL isFirstDraw;
}

- (id) initWithPluginInstance:(NPAPIPluginMacOS*) instance;

+ (void) getThreadChildContext:(CGLContextObj*) childContext;

@end

