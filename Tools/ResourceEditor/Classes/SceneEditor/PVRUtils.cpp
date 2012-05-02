#include "PVRUtils.h"

using namespace DAVA;

#define PVR_TEXTURE_FLAG_TYPE_MASK   0xff

enum
{
    kPVRTextureFlagTypePVRTC_2 = 24,
    kPVRTextureFlagTypePVRTC_4
};


PVRUtils::PVRUtils()
{
}

PVRUtils::~PVRUtils()
{

}

uint32 PVRUtils::ConvertLittleToHost(uint32 value)
{
    uint32 testValue = 1;
    uint8 *testValuePtr = (uint8 *)&testValue;
    
    if(testValuePtr[0] < testValuePtr[1])
    {   //be
        uint8 *valuePtr = (uint8 *)&value;
        
        testValuePtr[0] = valuePtr[3];
        testValuePtr[1] = valuePtr[2];
        testValuePtr[2] = valuePtr[1];
        testValuePtr[3] = valuePtr[0];
        
        return testValue;
    }
    
    return value;    
}

bool PVRUtils::GetPVRHeader(PVRHeader *header, const String &path)
{
    File *pvrFile = File::Create(path, File::READ | File::OPEN);
    if(pvrFile)
    {
        int32 readBytes = pvrFile->Read(header, sizeof(PVRHeader));
        SafeRelease(pvrFile);
        
        header->headerLength = ConvertLittleToHost(header->headerLength);
        header->height = ConvertLittleToHost(header->height);
        header->width = ConvertLittleToHost(header->width);
        header->numMipmaps = ConvertLittleToHost(header->numMipmaps);
        header->flags = ConvertLittleToHost(header->flags);
        header->dataLength = ConvertLittleToHost(header->dataLength);
        header->bpp = ConvertLittleToHost(header->bpp);
        header->bitmaskRed = ConvertLittleToHost(header->bitmaskRed);
        header->bitmaskGreen = ConvertLittleToHost(header->bitmaskGreen);
        header->bitmaskBlue = ConvertLittleToHost(header->bitmaskBlue);
        header->bitmaskAlpha = ConvertLittleToHost(header->bitmaskAlpha);
        header->pvrTag = ConvertLittleToHost(header->pvrTag);
        header->numSurfs = ConvertLittleToHost(header->numSurfs);
        
        return (sizeof(PVRHeader) == readBytes);
    }
    else
    {
        Logger::Error("[PVRUtils::GetPVRHeader] can't read pvr: %s", path.c_str());
    }
    return false;
}

PixelFormat PVRUtils::GetPVRFormat(int32 format)
{
    PixelFormat retFormat = FORMAT_INVALID;
    uint32 formatFlags = format & PVR_TEXTURE_FLAG_TYPE_MASK;
    if(kPVRTextureFlagTypePVRTC_4 == formatFlags)
    {
        retFormat = FORMAT_PVR4;
    }
    else if(kPVRTextureFlagTypePVRTC_2 == formatFlags)
    {
        retFormat = FORMAT_PVR2;
    }
    
    return retFormat;
}

uint32 PVRUtils::GetPVRDataLength(const String &path)
{
    PVRHeader header;
    bool ret = GetPVRHeader(&header, path);
    if(ret)
    {
        return header.dataLength;
    }
    
    return 0;
}

PixelFormat PVRUtils::GetPVRFormat(const String &path)
{
    PVRHeader header;
    bool ret = GetPVRHeader(&header, path);
    if(ret)
    {
        return GetPVRFormat(header.flags);
    }
    
    return FORMAT_INVALID;
}

WideString PVRUtils::SizeInBytesToWideString(float32 size)
{
    return StringToWString(SizeInBytesToString(size));
}

String PVRUtils::SizeInBytesToString(float32 size)
{
    String retString = "";
    
    if(1000000 < size)
    {
        retString = Format("%0.2f MB", size / (1024 * 1024) );
    }
    else if(1000 < size)
    {
        retString = Format("%0.2f KB", size / 1024);
    }
    else 
    {
        retString = Format("%d B", (int32)size);
    }
    
    return  retString;
}

