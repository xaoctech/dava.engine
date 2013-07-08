//
//  NPAPICorePlatformMacOS.cpp
//  Framework
//
//  Created by Yuri Coder on 5/22/13.
//
//

#include "NPAPICorePlatformMacOS.h"

namespace DAVA {
	
NPAPICoreMacOSPlatform::NPAPICoreMacOSPlatform()
{
}

NPAPICoreMacOSPlatform::~NPAPICoreMacOSPlatform()
{
}

Core::eScreenMode NPAPICoreMacOSPlatform::GetScreenMode()
{
    return Core::MODE_WINDOWED;
}

void NPAPICoreMacOSPlatform::ToggleFullscreen()
{
	Logger::Error("Toggle Full Screen is not supported on NPAPI Plugin");
}

void NPAPICoreMacOSPlatform::SwitchScreenToMode(eScreenMode screenMode)
{
    Logger::Error("[NPAPICoreMacOSPlatform::SwitchScreenToMode] is not supported on NPAPI Plugin.");
}

void* NPAPICoreMacOSPlatform::GetOpenGLView()
{
	// No OpenGL View exists on the NPAPI Plugin - layer only.
	return NULL;
}


// some macros to make code more readable.
#define GetModeWidth(mode) GetDictionaryLong((mode), kCGDisplayWidth)
#define GetModeHeight(mode) GetDictionaryLong((mode), kCGDisplayHeight)
#define GetModeRefreshRate(mode) GetDictionaryLong((mode), kCGDisplayRefreshRate)
#define GetModeBitsPerPixel(mode) GetDictionaryLong((mode), kCGDisplayBitsPerPixel)

//------------------------------------------------------------------------------------------
long NPAPICoreMacOSPlatform::GetDictionaryLong(CFDictionaryRef theDict, const void* key)
{
	// get a long from the dictionary
	long value = 0;
	CFNumberRef numRef;
	numRef = (CFNumberRef)CFDictionaryGetValue(theDict, key);
	if (numRef != NULL)
		CFNumberGetValue(numRef, kCFNumberLongType, &value);
	return value;
}

DisplayMode NPAPICoreMacOSPlatform::GetCurrentDisplayMode()
{
	CFDictionaryRef currentMode = CGDisplayCurrentMode(kCGDirectMainDisplay);
	
	DisplayMode mode;
	mode.width = GetModeWidth(currentMode);
	mode.height = GetModeHeight(currentMode);
	mode.refreshRate = GetModeRefreshRate(currentMode);
	mode.bpp = GetModeBitsPerPixel(currentMode);
	
	return mode;
}

void NPAPICoreMacOSPlatform::GetAvailableDisplayModes(List<DisplayMode> & availableModes)
{
	// One and only Display Mode is available on NPAPI plugin.
	availableModes.push_back(GetCurrentDisplayMode());
}

};
