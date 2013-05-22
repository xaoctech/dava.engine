//
//  NPAPIPluginMacOS.h
//  Framework
//
//  Created by Yuri Coder on 5/22/13.
//
//

#include "npapisdk/headers/npapi.h"
#include "npapisdk/headers/npfunctions.h"
#include "ApplicationCore.h"

#import <Foundation/Foundation.h>
#import "NPAPIOpenGLLayerMacOS.h"

// An interface to handle calls from NPAPI.
@interface NPAPIPluginMacOS : NSObject
{
	NPP npp;
	NPWindow* npWindow;
	NPAPIOpenGLLayerMacOS* openGLLayer;
	
	DAVA::ApplicationCore* appCore;
	
	// Whether browser window has focus?
	BOOL hasFocused;
	
	// Bundle Path to the Plugin (passed from the HTML page).
	NSString* bundlePath;
}

// Constructor/destructor.
-(NPAPIPluginMacOS*) initWithNPP:(NPP)instance;
-(void) dealloc;

// Initialize with parameters sent from the plugin.
-(NPError) npNew:(NPMIMEType)pluginType npMode:(uint16_t) mode npArgCount:(int16_t)argCount
	npArgNames:(char*[])argNames npArgValues:(char*[]) argValues npSavedData:(NPSavedData*) savedData;

// Set the NPWindow.
-(NPError) npSetWindow:(NPWindow*) window;

// Handle the Cocoa event came from browser.
-(NPError) npHandleEvent:(NPCocoaEvent*) event;

// Set the Bundle Path to the resources.
-(void) setBundlePath:(NSString*) value;

// Called when the browser requests us to return a value.
-(NPError) npGetValue:(NPPVariable) variable withParam:(void*)param;

// Called by OpenGL Layer to perform initialization on the first draw.
-(void) doInitializationOnFirstDraw;

// Event handlers
-(NPError) handleWindowFocusChanged:(BOOL) newValue;
- (void) onSuspend;
- (void) onResume;


@end
