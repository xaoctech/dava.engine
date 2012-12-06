#ifndef __DXT_CONVERTER_H__
#define __DXT_CONVERTER_H__

#include "DAVAEngine.h"

class DXTConverter: public DAVA::Singleton<DXTConverter>
{    
public:
 
	DXTConverter();
	virtual ~DXTConverter();

	
	DAVA::String ConvertPngToDxt(const DAVA::String & fileToConvert, const DAVA::TextureDescriptor &descriptor);

	
	void SetDXTTexTool(const DAVA::String &textToolPathname);

	
	DAVA::String GetDXTToolOutput(const DAVA::String &inputDXT);

protected:

	DAVA::Map<DAVA::PixelFormat, DAVA::String> pixelFormatToDXTFormat;

};

#endif // __DXT_CONVERTER_H__