#ifndef __PVR_CONVERTER_H__
#define __PVR_CONVERTER_H__

#include "DAVAEngine.h"

class PVRConverter: public DAVA::Singleton<PVRConverter>
{    
public:
 
	PVRConverter();
	virtual ~PVRConverter();

    DAVA::String ConvertPngToPvr(const DAVA::String & fileToConvert, DAVA::PixelFormat format, bool generateMimpaps);
	DAVA::String ConvertPngToPvr(const DAVA::String & fileToConvert, const DAVA::TextureDescriptor &descriptor);

	DAVA::String GetCommandLinePVR(const DAVA::String & fileToConvert, const DAVA::TextureDescriptor &descriptor);
	//DAVA::String GetCommandLineDXT(const DAVA::String & fileToConvert);

protected:
	DAVA::Map<DAVA::PixelFormat, DAVA::String> pixelFormatToPVRFormat;
	//DAVA::Map<DAVA::PixelFormat, DAVA::String> pixelFormatToDVXFormat;
	DAVA::String dataFolderPath;

	DAVA::String GetPVRToolPath();
	DAVA::String GetPVRToolOutput(const DAVA::String &inputPVR);

	//DAVA::String GetRelativeToolPathDXT(const DAVA::String &basePath);
};



#endif // __PVR_CONVERTER_H__