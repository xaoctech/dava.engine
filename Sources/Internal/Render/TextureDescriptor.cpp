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
#include "FileSystem/DynamicMemoryFile.h"
#include "Render/Texture.h"

namespace DAVA
{
    
TextureDescriptor::TextureDescriptor()
{
    pathname = String("");
    
    SetDefaultValues();
}

TextureDescriptor::~TextureDescriptor()
{
#if defined TEXTURE_SPLICING_ENABLED
    SafeRelease(textureFile);
#endif //#if defined TEXTURE_SPLICING_ENABLED

}

void TextureDescriptor::SetDefaultValues()
{
    Memset(modificationDate, 0, DATE_BUFFER_SIZE * sizeof(char8));
    Memset(crc, 0, MD5::DIGEST_SIZE * sizeof(uint8));

    wrapModeS = Texture::WRAP_REPEAT;
    wrapModeT = Texture::WRAP_REPEAT;
    
    generateMipMaps = OPTION_ENABLED;
    
    pvrCompression.format = FORMAT_PVR4;
    pvrCompression.flipVertically = OPTION_DISABLED;
    pvrCompression.baseMipMapLevel = 0;
    
    dxtCompression.format = FORMAT_INVALID;
    dxtCompression.flipVertically = OPTION_DISABLED;
    dxtCompression.baseMipMapLevel = 0;
    
#if defined TEXTURE_SPLICING_ENABLED
    textureFile = NULL;
#endif //#if defined TEXTURE_SPLICING_ENABLED

    textureFileFormat = Texture::PNG_FILE;
}
    
void TextureDescriptor::SaveDateAndCrc(const String &filePathname)
{
    const char8 *date = File::GetModificationDate(filePathname);
    if(date)
    {
        strncpy(modificationDate, date, DATE_BUFFER_SIZE-1);
        modificationDate[DATE_BUFFER_SIZE-1] = 0;
        
        MD5::ForFile(filePathname, crc);
    }
    else
    {
        Memset(modificationDate, 0, DATE_BUFFER_SIZE * sizeof(char8));
        Memset(crc, 0, MD5::DIGEST_SIZE * sizeof(uint8));
    }
}

    
bool TextureDescriptor::Load(const String &filePathname)
{
    File *file = File::Create(filePathname, File::READ | File::OPEN);
    if(!file)
    {
        Logger::Error("[TextureDescriptor::Load] Can't open file: %s", filePathname.c_str());
        return false;
    }
    
    pathname = filePathname;
    
    int32 signature;
    file->Read(&signature, sizeof(signature));
    
    if(COMPRESSED_FILE == signature)
    {
        LoadCompressed(file);
    }
    else
    {
        LoadNotCompressed(file);
    }
    
    SafeRelease(file);
    
    return true;
}

void TextureDescriptor::Save()
{
    DVASSERT(!pathname.empty() && "Can use this method only after calling Load()");
    Save(pathname);
}
    
void TextureDescriptor::Save(const String &filePathname)
{
    File *file = File::Create(filePathname, File::WRITE | File::OPEN | File::CREATE);
    if(!file)
    {
        Logger::Error("[TextureDescriptor::Save] Can't open file: %s", filePathname.c_str());
        return;
    }
    
    int32 signature = NOTCOMPRESSED_FILE;
    file->Write(&signature, sizeof(signature));
    
    //date
    file->Write(modificationDate, DATE_BUFFER_SIZE * sizeof(char8));

    //crc
    char8 readableCrc[MD5::DIGEST_SIZE*2 + 1];
    CrcToReadableFormat(readableCrc, MD5::DIGEST_SIZE*2 + 1);

    file->Write(readableCrc, (MD5::DIGEST_SIZE*2 + 1) * sizeof(char8));

    WriteGeneralSettings(file);
    
    //Compression
    WriteCompression(file, pvrCompression);
    WriteCompression(file, dxtCompression);
    
    SafeRelease(file);
}
    
void TextureDescriptor::Export(const String &filePathname, const String &texturePathname)
{
    File *file = File::Create(filePathname, File::WRITE | File::OPEN | File::CREATE);
    if(!file)
    {
        Logger::Error("[TextureDescriptor::Save] Can't open file: %s", filePathname.c_str());
        return;
    }

    int32 signature = COMPRESSED_FILE;
    file->Write(&signature, sizeof(signature));

    WriteGeneralSettings(file);
    file->Write(&textureFileFormat, sizeof(textureFileFormat));

#if defined TEXTURE_SPLICING_ENABLED
    SafeRelease(textureFile);
    textureFile = File::Create(texturePathname, File::OPEN | File::READ);
    if(textureFile)
    {
        uint32 fileSize = textureFile->GetSize();
        uint8 *fileData = new uint8[fileSize];
        if(fileData)
        {
            textureFile->Read(fileData, fileSize);
            file->Write(fileData, fileSize);
            
            SafeDeleteArray(fileData);
        }
        
        SafeRelease(textureFile);
    }
#endif //#if defined TEXTURE_SPLICING_ENABLED

    SafeRelease(file);
}
    
    
void TextureDescriptor::LoadNotCompressed(File *file)
{
    file->Read(modificationDate, DATE_BUFFER_SIZE * sizeof(char8));
    
    char8 readableCrc[MD5::DIGEST_SIZE*2 + 1];
    file->Read(readableCrc, (MD5::DIGEST_SIZE*2 + 1) * sizeof(char8));
    CrcFromReadableFormat(readableCrc);
    
    ReadGeneralSettings(file);
    
    ReadCompression(file, pvrCompression);
    ReadCompression(file, dxtCompression);
}
    
void TextureDescriptor::LoadCompressed(File *file)
{
#if defined TEXTURE_SPLICING_ENABLED
    SafeRelease(textureFile);
#endif //#if defined TEXTURE_SPLICING_ENABLED

    ReadGeneralSettings(file);
    file->Read(&textureFileFormat, sizeof(textureFileFormat));

#if defined TEXTURE_SPLICING_ENABLED
    uint32 textureSize = file->GetSize() - file->GetPos();
    uint8 *textureData = new uint8[textureSize];
    if(textureData)
    {
        file->Read(textureData, textureSize);

        textureFile = DynamicMemoryFile::Create(textureData, textureSize, File::WRITE | File::READ);
        SafeDeleteArray(textureData);
    }
#endif //#if defined TEXTURE_SPLICING_ENABLED

}

void TextureDescriptor::ReadGeneralSettings(File *file)
{
    file->Read(&wrapModeS, sizeof(wrapModeS));
    file->Read(&wrapModeT, sizeof(wrapModeT));
    file->Read(&generateMipMaps, sizeof(generateMipMaps));
}
    
void TextureDescriptor::WriteGeneralSettings(File *file)
{
    file->Write(&wrapModeS, sizeof(wrapModeS));
    file->Write(&wrapModeT, sizeof(wrapModeT));
    file->Write(&generateMipMaps, sizeof(generateMipMaps));
}

    
void TextureDescriptor::ReadCompression(File *file, Compression &compression)
{
    int8 format;
    file->Read(&format, sizeof(format));
    compression.format = (PixelFormat)format;
    
    file->Read(&compression.flipVertically, sizeof(compression.flipVertically));
    file->Read(&compression.baseMipMapLevel, sizeof(compression.baseMipMapLevel));
}

void TextureDescriptor::WriteCompression(File *file, const Compression &compression)
{
    int8 format = compression.format;
    file->Write(&format, sizeof(format));
    file->Write(&compression.flipVertically, sizeof(compression.flipVertically));
    file->Write(&compression.baseMipMapLevel, sizeof(compression.baseMipMapLevel));
}


void TextureDescriptor::CrcFromReadableFormat(const char8 *readCrc)
{
    int32 crcSize = strlen(readCrc);
    if((MD5::DIGEST_SIZE * 2) != crcSize)
    {
        Logger::Error("[TextureDescriptor::CrcFromReadableFormat] crc has wrong size (%d). Must be 32 characters", crcSize);
        return;
    }
    
    for(int32 i = 0; i < MD5::DIGEST_SIZE; ++i)
    {
        uint8 low = GetNumberFromCharacter(readCrc[2*i]);
        uint8 high = GetNumberFromCharacter(readCrc[2*i + 1]);

        crc[i] = (high << 4) | (low);
    }
}

void TextureDescriptor::CrcToReadableFormat(char8 *readCrc, int32 crcSize)
{
    DVASSERT(((MD5::DIGEST_SIZE*2 + 1) <= crcSize) && "To small buffer. Must be enought to put 32 characters of crc and \0");

    for(int32 i = 0; i < MD5::DIGEST_SIZE; ++i)
    {
        readCrc[2*i] = GetCharacterFromNumber(crc[i] & 0x0F);
        readCrc[2*i + 1] = GetCharacterFromNumber( (crc[i] & 0xF0) >> 4 );
    }
    
    readCrc[2 * MD5::DIGEST_SIZE] = 0;
}
    
uint8 TextureDescriptor::GetNumberFromCharacter(char8 character)
{
    if('0' <= character && character <= '9')
    {
        return (character - '0');
    }
    else if('a' <= character && character <= 'f')
    {
        return (character - 'a' + 10);
    }
    else if('A' <= character && character <= 'F')
    {
        return (character - 'A' + 10);
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

    return (number + 'A' - 10);
}
    
bool TextureDescriptor::GetGenerateMipMaps()
{
    return (OPTION_DISABLED != generateMipMaps);
}
    

String TextureDescriptor::GetDefaultExtension()
{
    return String(".tex");
}
    
    
};
