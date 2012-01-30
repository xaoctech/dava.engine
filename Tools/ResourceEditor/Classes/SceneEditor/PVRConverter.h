#ifndef __PVR_CONVERTER_H__
#define __PVR_CONVERTER_H__

#include "DAVAEngine.h"

class PVRConverter: public DAVA::Singleton<PVRConverter>
{    
public:
 
	PVRConverter();
	virtual ~PVRConverter();

	void ConvertPvrToPng(const DAVA::String & fileToConvert);

protected:

	DAVA::String dataFolderPath;
};



#endif // __PVR_CONVERTER_H__