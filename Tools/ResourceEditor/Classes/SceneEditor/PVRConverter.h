#ifndef __PVR_CONVERTER_H__
#define __PVR_CONVERTER_H__

#include "DAVAEngine.h"

class PVRConverter: public DAVA::Singleton<PVRConverter>
{    
public:
 
	PVRConverter();
	virtual ~PVRConverter();

	DAVA::String ConvertPngToPvr(const DAVA::String & fileToConvert, const DAVA::TextureDescriptor &descriptor);

	DAVA::String GetCommandLinePVR(const DAVA::String & fileToConvert, const DAVA::TextureDescriptor &descriptor);

	void SetPVRTexTool(const DAVA::String &textToolPathname);

	DAVA::String GetPVRToolOutput(const DAVA::String &inputPVR);

protected:
	DAVA::Map<DAVA::PixelFormat, DAVA::String> pixelFormatToPVRFormat;

	DAVA::String pvrTexToolPathname;
};

#endif // __PVR_CONVERTER_H__