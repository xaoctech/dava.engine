#ifndef __PVR_UTILS_H__
#define __PVR_UTILS_H__

#include "DAVAEngine.h"

using namespace DAVA;

class PVRUtils: public DAVA::Singleton<PVRUtils>
{    
public:
    
    typedef struct _PVRHeader
    {
        uint32 headerLength;
        uint32 height;
        uint32 width;
        uint32 numMipmaps;
        uint32 flags;
        uint32 dataLength;
        uint32 bpp;
        uint32 bitmaskRed;
        uint32 bitmaskGreen;
        uint32 bitmaskBlue;
        uint32 bitmaskAlpha;
        uint32 pvrTag;
        uint32 numSurfs;
    } PVRHeader;
    
public:
 
	PVRUtils();
	virtual ~PVRUtils();

    bool GetPVRHeader(PVRHeader *header, const String &path);
    PixelFormat GetPVRFormat(int32 format);

    PixelFormat GetPVRFormat(const String &path);
    uint32 GetPVRDataLength(const String &path);
    
    static WideString SizeInBytesToWideString(float32 size);
    static String SizeInBytesToString(float32 size);

    
protected:


    uint32 ConvertLittleToHost(uint32 value);

};



#endif // __PVR_CONVERTER_H__