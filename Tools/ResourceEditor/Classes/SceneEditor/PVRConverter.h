#ifndef __PVR_CONVERTER_H__
#define __PVR_CONVERTER_H__

#include "DAVAEngine.h"

class PVRConverter: public DAVA::Singleton<PVRConverter>
{    
public:
 
	PVRConverter();
	virtual ~PVRConverter();

	DAVA::FilePath ConvertPngToPvr(const DAVA::FilePath & fileToConvert, const DAVA::TextureDescriptor &descriptor);

	DAVA::String GetCommandLinePVR(const DAVA::FilePath & fileToConvert, const DAVA::TextureDescriptor &descriptor);

	void SetPVRTexTool(const DAVA::FilePath &textToolPathname);

	DAVA::FilePath GetPVRToolOutput(const DAVA::FilePath &inputPVR);

protected:
	DAVA::Map<DAVA::PixelFormat, DAVA::String> pixelFormatToPVRFormat;

	DAVA::FilePath pvrTexToolPathname;
};

#endif // __PVR_CONVERTER_H__