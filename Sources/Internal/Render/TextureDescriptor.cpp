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
=====================================================================================*/
#include "Render/TextureDescriptor.h"
#include "FileSystem/Logger.h"
#include "FileSystem/File.h"
#include "Utils/MD5.h"
#include "Render/Texture.h"

namespace DAVA
{
	
TextureDescriptor::TextureDescriptor()
{
    SetDefaultValues();
}

TextureDescriptor::~TextureDescriptor()
{
    
}

void TextureDescriptor::SetDefaultValues()
{
    Memset(crc, 0, MD5_BUFFER_SIZE * sizeof(uint8));

    wrapModeS = WRAP_CLAMP_TO_EDGE;
    wrapModeT = WRAP_CLAMP_TO_EDGE;
    
    generateMipMaps = false;
	isAlphaPremultiplied = false;
    
    pvrCompression.format = FORMAT_PVR4;
    pvrCompression.flipVertically = false;
    pvrCompression.generateMipMaps = true;
    
    dxtCompression.format = FORMAT_INVALID;
    dxtCompression.flipVertically = false;
    dxtCompression.generateMipMaps = true;
}
    
    
void TextureDescriptor::Load(const String &filePathname)
{
    File *file = File::Create(filePathname, File::READ | File::OPEN);
    if(!file)
    {
        Logger::Error("[TextureDescriptor::Load] Can't open file: %s", filePathname.c_str());
        return;
    }
    
    //crc
    char8 readableCrc[MD5_STRING_SIZE];
    ReadChar8String(file, readableCrc, MD5_STRING_SIZE);
    CrcFromReadableFormat(readableCrc);

    //Read WrapModes
    int32 wrapS = 0;
    ReadInt32(file, wrapS);
    wrapModeS = (TextureWrap)wrapS;

    int32 wrapT = 0;
    ReadInt32(file, wrapT);
    wrapModeT = (TextureWrap)wrapT;
    

    //Read isMipMapTexture
    int32 mipmap = 0;
    ReadInt32(file, mipmap);
    generateMipMaps = (0 != mipmap);

    //Read isAlphaPremultiplied
    int32 alpha = 0;
    ReadInt32(file, alpha);
    isAlphaPremultiplied = (0 != alpha);

    //Compression
    ReadCompression(file, pvrCompression);
    ReadCompression(file, dxtCompression);
    
    SafeRelease(file);
}

void TextureDescriptor::Save(const String &filePathname)
{
    File *file = File::Create(filePathname, File::WRITE | File::OPEN | File::CREATE);
    if(!file)
    {
        Logger::Error("[TextureDescriptor::Save] Can't open file: %s", filePathname.c_str());
        return;
    }
    
    //crc
    char8 readableCrc[MD5_STRING_SIZE];
    CrcToReadableFormat(readableCrc, MD5_STRING_SIZE);
    WriteChar8String(file, readableCrc);

    WriteInt32(file, (int32)wrapModeS);
    WriteInt32(file, (int32)wrapModeT);

    WriteInt32(file, (generateMipMaps) ? 1: 0);
    WriteInt32(file, (isAlphaPremultiplied) ? 1: 0);

    //Compression
    WriteCompression(file, pvrCompression);
    WriteCompression(file, dxtCompression);
    
    SafeRelease(file);
}
    
void TextureDescriptor::ReadCompression(File *file, Compression &compression)
{
    char8 formatName[MD5_STRING_SIZE];
    ReadChar8String(file, formatName, MD5_STRING_SIZE);
    compression.format = Texture::GetPixelFormatByName(String(formatName));
    
    int32 mipmap = 0;
    ReadInt32(file, mipmap);
    compression.generateMipMaps = (0 != mipmap);
    
    int32 flip = 0;
    ReadInt32(file, flip);
    compression.flipVertically = (0 != flip);
}

void TextureDescriptor::WriteCompression(File *file, const Compression &compression)
{
    WriteChar8String(file, Texture::GetPixelFormatString(compression.format));
    WriteInt32(file, (compression.generateMipMaps) ? 1: 0);
    WriteInt32(file, (compression.flipVertically) ? 1: 0);
}

    
void TextureDescriptor::ReadInt32(DAVA::File *file, int32 &value)
{
    char8 lineData[LINE_SIZE];
    uint32 lineSize = file->ReadLine(lineData, LINE_SIZE);
    if(lineSize)
    {
        sscanf(lineData, "%d", &value);
    }
}
    
void TextureDescriptor::WriteInt32(DAVA::File *file, const int32 value)
{
    char8 lineData[LINE_SIZE];
    sprintf(lineData, "%d", value);
    file->WriteLine(lineData);
}
    
void TextureDescriptor::ReadChar8String(File *file, char8 *buffer, uint32 bufferSize)
{
    char8 lineData[LINE_SIZE];
    uint32 lineSize = file->ReadLine(lineData, Min((uint32)LINE_SIZE, bufferSize - 1));
    if(lineSize)
    {
        sscanf(lineData, "%s", buffer);
    }
    else
    {
        buffer[0] = 0;
    }
}
    
void TextureDescriptor::WriteChar8String(File *file, const char8 *buffer)
{
    char8 lineData[LINE_SIZE];
    sprintf(lineData, "%s", buffer);
    file->WriteLine(lineData);
}

void TextureDescriptor::CrcFromReadableFormat(const char8 *readCrc)
{
    int32 crcSize = strlen(readCrc);
    if((MD5_BUFFER_SIZE * 2) != crcSize)
    {
        Logger::Error("[TextureDescriptor::CrcFromReadableFormat] crc has wrong size (%d). Must be 32 characters", crcSize);
        return;
    }
    
    for(int32 i = 0; i < MD5_BUFFER_SIZE; ++i)
    {
        uint8 low = GetNumberFromCharacter(readCrc[2*i]);
        uint8 high = GetNumberFromCharacter(readCrc[2*i + 1]);

        crc[i] = (high << 4) | (low);
    }
}

void TextureDescriptor::CrcToReadableFormat(char8 *readCrc, int32 crcSize)
{
    DVASSERT((MD5_STRING_SIZE <= crcSize) && "To small buffer. Must be enought to put 32 characters of crc and \0");

    for(int32 i = 0; i < MD5_BUFFER_SIZE; ++i)
    {
        readCrc[2*i] = GetCharacterFromNumber(crc[i] & 0x0F);
        readCrc[2*i + 1] = GetCharacterFromNumber( (crc[i] & 0xF0) >> 4 );
    }
    
    readCrc[2 * MD5_BUFFER_SIZE] = 0;
}
    
    uint8 TextureDescriptor::GetNumberFromCharacter(char8 character)
    {
        if('0' <= character && character <= '9')
        {
            return (character - '0');
        }
        else if('a' <= character && character <= 'f')
        {
            return (character - 'a');
        }
        else if('A' <= character && character <= 'F')
        {
            return (character - 'A');
        }
        
        Logger::Error("[TextureDescriptor::CrcFromReadableFormat] crc has wrong symbol (%c).", character);
        return 0;
}

char8 TextureDescriptor::GetCharacterFromNumber(uint8 number)
{
    if(0 <= number && number <= 9)
    {
        return (number + '0');
    }

    return (number + 'A');
}

    
};
