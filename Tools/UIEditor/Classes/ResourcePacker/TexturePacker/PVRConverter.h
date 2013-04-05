#ifndef __PVR_CONVERTER_H__
#define __PVR_CONVERTER_H__

#include "DAVAEngine.h"

class PVRConverter: public DAVA::Singleton<PVRConverter>
{    
public:
 
	PVRConverter();
	virtual ~PVRConverter();

	//DAVA::String ConvertPngToPvr(const DAVA::String & fileToConvert, DAVA::PixelFormat format, bool generateMimpaps);
	DAVA::String ConvertPngToPvr(const DAVA::FilePath & fileToConvert, const DAVA::TextureDescriptor &descriptor);

	DAVA::String GetCommandLinePVR(const DAVA::FilePath & fileToConvert, const DAVA::TextureDescriptor &descriptor);
	//DAVA::String GetCommandLineDXT(const DAVA::String & fileToConvert);

	void SetPVRTexTool(const DAVA::FilePath &textToolPathname);
	void SetDXTTexTool(const DAVA::FilePath &textToolPathname);

	DAVA::String GetPVRToolOutput(const DAVA::FilePath &inputPVR);
	//DAVA::String GetDXTToolOutput(const DAVA::String &inputDXT);

protected:
	DAVA::Map<DAVA::PixelFormat, DAVA::String> pixelFormatToPVRFormat;
	//DAVA::Map<DAVA::PixelFormat, DAVA::String> pixelFormatToDVXFormat;

	DAVA::FilePath pvrTexToolPathname;
	DAVA::FilePath dxtTexToolPathname;
};

#endif // __PVR_CONVERTER_H__