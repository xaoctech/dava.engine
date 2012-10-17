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
    enum eOptionsState
    {
        OPTION_DISABLED = 0,
        OPTION_ENABLED = 1,
    };

    enum eSignatures
    {
        COMPRESSED_FILE = 0x00EEEE00,
        NOTCOMPRESSED_FILE = 0x00EE00EE
    };
    
    static const int32 DATE_BUFFER_SIZE = 20;
    static const int32 LINE_SIZE = 256;

public:
    
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

    void Save();
    void Save(const String &filePathname);
    
    void Export(const String &filePathname, const String &texturePathname);

    bool GetGenerateMipMaps();

    static String GetDefaultExtension();
    
protected:
    
    void LoadNotCompressed(File *file);
    void LoadCompressed(File *file);
    
    void SetDefaultValues();
    
    void ReadGeneralSettings(File *file);
    void WriteGeneralSettings(File *file);
    
    void ReadCompression(File *file, Compression &compression);
    void WriteCompression(File *file, const Compression &compression);
    
public:
    
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
    
    String pathname;
};
    
};
#endif // __DAVAENGINE_TEXTURE_DESCRIPTOR_H__
