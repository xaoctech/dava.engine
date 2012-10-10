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

namespace DAVA
{

class File;
class TextureDescriptor: public BaseObject
{
    enum eConst
    {
        MD5_BUFFER_SIZE = 16,
        MD5_STRING_SIZE = MD5_BUFFER_SIZE * 2 + 1,
        LINE_SIZE = 256
    };
    
public:
	enum TextureWrap
	{
		WRAP_CLAMP_TO_EDGE = 0,
		WRAP_REPEAT,
	};
    
    struct Compression
    {
        PixelFormat format;
        bool generateMipMaps;
        bool flipVertically;
    };


public:
    TextureDescriptor();
    virtual ~TextureDescriptor();
    
    void Load(const String &filePathname);
    void Save(const String &filePathname);
    
protected:
    void SetDefaultValues();
    void CrcFromReadableFormat(const char8 *readCrc);
    void CrcToReadableFormat(char8 *readCrc, int32 crcSize);
    
    uint8 GetNumberFromCharacter(char8 character);
    char8 GetCharacterFromNumber(uint8 number);
    
    void ReadInt32(File *file, int32 &value);
    void WriteInt32(File *file, const int32 value);

    void ReadChar8String(File *file, char8 *buffer, uint32 bufferSize);
    void WriteChar8String(File *file, const char8 *buffer);
    
    void ReadCompression(File *file, Compression &compression);
    void WriteCompression(File *file, const Compression &compression);
    
public:
    
    uint8 crc[MD5_BUFFER_SIZE];

    TextureWrap wrapModeS;
    TextureWrap wrapModeT;
    
    bool generateMipMaps;
	bool isAlphaPremultiplied;
    
    Compression pvrCompression;
    Compression dxtCompression;
};
    
};
#endif // __DAVAENGINE_TEXTURE_DESCRIPTOR_H__
