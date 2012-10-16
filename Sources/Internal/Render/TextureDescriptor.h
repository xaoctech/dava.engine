/*==================================================================================
    Copyright (c) 2008, DAVA Consulting, LLC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA Consulting, LLC nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    Revision History:
        * Created by Vitaliy Borodovsky 
        \todo Add possibility to change premultiplication & make possibility to change format on fly
=====================================================================================*/
#ifndef __DAVAENGINE_TEXTURE_DESCRIPTOR_H__
#define __DAVAENGINE_TEXTURE_DESCRIPTOR_H__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "Utils/MD5.h"

//#define TEXTURE_SPLICING_ENABLED

namespace DAVA
{

class File;
class TextureDescriptor: public BaseObject
{
    enum eConst
    {
        OPTION_DISABLED = 0,
        OPTION_ENABLED = 1,
        
        BINARY_SIGNATURE = 0x006E6962,
        TEXT_SIGNATURE = 0x00747874,
        
        DATE_BUFFER_SIZE = 20,
        LINE_SIZE = 256
    };
    
public:
    
    enum eFileType
    {
        TYPE_TEXT = 0,
        TYPE_BINARY
    };
    
    enum TextureFileFormat
    {
        PNG_FILE = 0,
        PVR_FILE,
        DXT_FILE
    };

    struct Compression
    {
        PixelFormat format;
        int8 flipVertically;
        int8 baseMipMapLevel;
    };


public:
    TextureDescriptor();
    virtual ~TextureDescriptor();
    
    void SaveDateAndCrc(const String &filePathname);

    bool Load(const String &filePathname);
    void SaveAsText(const String &filePathname);
    void SaveAsBinary(const String &filePathname, const String &texturePathname);

    bool GenerateMipMaps();

    static String GetDefaultExtension();
    
protected:
    
    eFileType DetectFileType(File *file);
    
    void LoadAsText(File *file);
    void LoadAsBinary(File *file);
    
    void SetDefaultValues();
    void CrcFromReadableFormat(const char8 *readCrc);
    void CrcToReadableFormat(char8 *readCrc, int32 crcSize);
    
    uint8 GetNumberFromCharacter(char8 character);
    char8 GetCharacterFromNumber(uint8 number);
    
    void ReadInt8(File *file, int8 &value);
    void WriteInt8(File *file, const int8 value);

    void ReadChar8String(File *file, char8 *buffer, uint32 bufferSize);
    void WriteChar8String(File *file, const char8 *buffer);
    
    void ReadCompressionAsText(File *file, Compression &compression);
    void WriteCompressionAsText(File *file, const Compression &compression);
    
    void WriteSignature(File *file, int32 signature);
    
public:
    
    eFileType fileType;
    
    char8 modificationDate[DATE_BUFFER_SIZE];
    uint8 crc[MD5::DIGEST_SIZE];

    int8 wrapModeS;
    int8 wrapModeT;
    
    int8 generateMipMaps;
    
    Compression pvrCompression;
    Compression dxtCompression;
    
    //Binary only
    int8 textureFileFormat;
#if defined TEXTURE_SPLICING_ENABLED
    File *textureFile;
#endif //#if defined TEXTURE_SPLICING_ENABLED
};
    
};
#endif // __DAVAENGINE_TEXTURE_DESCRIPTOR_H__
