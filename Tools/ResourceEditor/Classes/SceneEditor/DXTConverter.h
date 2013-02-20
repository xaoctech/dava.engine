#ifndef __DXT_CONVERTER_H__
#define __DXT_CONVERTER_H__

#include "DAVAEngine.h"

class DXTConverter
{    
public:
 
	static DAVA::String ConvertPngToDxt(const DAVA::String & fileToConvert, const DAVA::TextureDescriptor &descriptor);

	static DAVA::String GetDXTOutput(const DAVA::String &inputDXT);
};

#endif // __DXT_CONVERTER_H__