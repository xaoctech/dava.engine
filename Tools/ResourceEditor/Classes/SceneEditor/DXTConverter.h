#ifndef __DXT_CONVERTER_H__
#define __DXT_CONVERTER_H__

#include "DAVAEngine.h"

class DXTConverter
{    
public:
 
	static DAVA::FilePath ConvertPngToDxt(const DAVA::FilePath & fileToConvert, const DAVA::TextureDescriptor &descriptor);

	static DAVA::FilePath GetDXTOutput(const DAVA::FilePath &inputDXT);
};

#endif // __DXT_CONVERTER_H__