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
    wrapModeS = WRAP_CLAMP_TO_EDGE;
    wrapModeT = WRAP_CLAMP_TO_EDGE;
    
    isMipMapTexture = false;
	isAlphaPremultiplied = false;
    
    Memset(crc, 0, MD5_BUFFER_SIZE * sizeof(uint8));
    
    pvrCompressionFormat = FORMAT_PVR4;
}
    
    
void TextureDescriptor::Load(const String &filePathname)
{
    File *file = File::Create(filePathname, File::READ | File::OPEN);
    if(!file)
    {
        Logger::Error("[TextureDescriptor::Load] Can't open file: %s", filePathname.c_str());
        return;
    }
    
    uint32 fileDataSize = file->GetSize();
    char8 *lineData = new char8[fileDataSize];
    
    if(!lineData)
    {
        Logger::Error("[TextureDescriptor::Load] Can't allocate memory to read file: %s", filePathname.c_str());
        SafeRelease(file);
        return;
    }

    //Read WrapModes
    uint32 lineSize = file->ReadLine(lineData, fileDataSize);
    if(lineSize)
    {
        int32 wrapS, wrapT;
        sscanf(lineData, "%d %d", &wrapS, &wrapT);
        wrapModeS = (TextureWrap)wrapS;
        wrapModeT = (TextureWrap)wrapT;
    }
    

    //Read isMipMapTexture
    lineSize = file->ReadLine(lineData, fileDataSize);
    if(lineSize)
    {
        int32 mipmap = 0;
        sscanf(lineData, "%d", &mipmap);
        isMipMapTexture = (0 != mipmap);
    }

    //Read isAlphaPremultiplied
    lineSize = file->ReadLine(lineData, fileDataSize);
    if(lineSize)
    {
        int32 alpha = 0;
        sscanf(lineData, "%d", &alpha);
        isAlphaPremultiplied = (0 != alpha);
    }

    //crc
    lineSize = file->ReadLine(lineData, fileDataSize);
    if(lineSize)
    {
        char8 readCrc[MD5_STRING_SIZE];
        sscanf(lineData, "%s", readCrc);
        CrcFromReadableFormat(readCrc);
    }
    
    //PVRCompression
    lineSize = file->ReadLine(lineData, fileDataSize);
    if(lineSize)
    {
        char8 formatName[MD5_STRING_SIZE];
        sscanf(lineData, "%s", formatName);
        pvrCompressionFormat = Texture::GetPixelFormatByName(String(formatName));
    }

    
    
    SafeDeleteArray(lineData);
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
    
    uint32 lineDataSize = 1024;
    char8 *lineData = new char8[lineDataSize];
    
    if(!lineData)
    {
        Logger::Error("[TextureDescriptor::Save] Can't allocate memory to write file: %s", filePathname.c_str());
        SafeRelease(file);
        return;
    }
    
    //Write WrapModes
    sprintf(lineData, "%d %d", (int32)wrapModeS, (int32)wrapModeT);
    file->WriteLine(lineData);
    
    //write isMipMapTexture
    sprintf(lineData, "%d", (isMipMapTexture) ? 1: 0);
    file->WriteLine(lineData);
    

    //write isAlphaPremultiplied
    sprintf(lineData, "%d", (isAlphaPremultiplied) ? 1: 0);
    file->WriteLine(lineData);
    
    //crc
    CrcToReadableFormat(lineData, lineDataSize);
    file->WriteLine(lineData);
    
    
    //PVRCompression
    sprintf(lineData, "%s", Texture::GetPixelFormatString(pvrCompressionFormat));
    file->WriteLine(lineData);
    
    
    SafeDeleteArray(lineData);
    SafeRelease(file);
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
