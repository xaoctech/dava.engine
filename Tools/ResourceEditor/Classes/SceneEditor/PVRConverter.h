#ifndef __PVR_CONVERTER_H__
#define __PVR_CONVERTER_H__

#include "DAVAEngine.h"

class PVRConverter: public DAVA::Singleton<PVRConverter>
{    
public:
 
	PVRConverter();
	virtual ~PVRConverter();

    DAVA::String ConvertPngToPvr(const DAVA::String & fileToConvert, DAVA::PixelFormat format, bool generateMimpaps);

    void SetPVRTexToolPathname(const DAVA::String &pathname);
    
protected:

	DAVA::String dataFolderPath;
    DAVA::String pvrTexToolPathname;
};



#endif // __PVR_CONVERTER_H__