//
//  NPAPICorePlatformMacOS.h
//  Framework
//
//  Created by Yuri Coder on 5/22/13.
//
//

#ifndef __DAVAENGINE_NPAPI_CORE_PLATFORM_MAC_OS_H__
#define __DAVAENGINE_NPAPI_CORE_PLATFORM_MAC_OS_H__

#include "DAVAEngine.h"
#include <ApplicationServices/ApplicationServices.h>

namespace DAVA {

class NPAPICoreMacOSPlatform : public Core
{
public:
	NPAPICoreMacOSPlatform();
	virtual ~NPAPICoreMacOSPlatform();

	virtual eScreenMode GetScreenMode();
	virtual void SwitchScreenToMode(eScreenMode screenMode);
	virtual void ToggleFullscreen();
		
	virtual void GetAvailableDisplayModes(List<DisplayMode> & availableModes);
	virtual DisplayMode GetCurrentDisplayMode();

	virtual void* GetOpenGLView();
	
protected:
	long GetDictionaryLong(CFDictionaryRef theDict, const void* key);
};

};

#endif // __DAVAENGINE_NPAPI_CORE_PLATFORM_MAC_OS_H__
