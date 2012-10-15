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
    fileType = TYPE_TEXT;
    
    
    Memset(modificationDate, 0, DATE_BUFFER_SIZE * sizeof(char8));
    Memset(crc, 0, MD5::DIGEST_SIZE * sizeof(uint8));

    wrapModeS = Texture::WRAP_REPEAT;
    wrapModeT = Texture::WRAP_REPEAT;
    
    generateMipMaps = OPTION_ENABLED;
    baseMipMapLevel = 0;
    
    pvrCompression.format = FORMAT_PVR4;
    pvrCompression.flipVertically = OPTION_DISABLED;
    
    dxtCompression.format = FORMAT_INVALID;
    dxtCompression.flipVertically = OPTION_DISABLED;
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
    
    fileType = DetectFileType(file);
    if(TYPE_BINARY == fileType)
    {
        LoadAsBinary(file);
    }
    else
    {
        LoadAsText(file);
    }
    
    SafeRelease(file);
    
    return true;
}

    
void TextureDescriptor::SaveAsText(const String &filePathname)
{
    File *file = File::Create(filePathname, File::WRITE | File::OPEN | File::CREATE);
    if(!file)
    {
        Logger::Error("[TextureDescriptor::Save] Can't open file: %s", filePathname.c_str());
        return;
    }
    
    WriteSignature(file, TEXT_SIGNATURE);
    
    //date
    WriteChar8String(file, modificationDate);
    
    
    //crc
    char8 readableCrc[MD5::DIGEST_SIZE*2 + 1];
    CrcToReadableFormat(readableCrc, MD5::DIGEST_SIZE*2 + 1);
    WriteChar8String(file, readableCrc);

    WriteInt8(file, wrapModeS);
    WriteInt8(file, wrapModeT);

    WriteInt8(file, generateMipMaps);
    WriteInt8(file, baseMipMapLevel);

    //Compression
    WriteCompressionAsText(file, pvrCompression);
    WriteCompressionAsText(file, dxtCompression);
    
    SafeRelease(file);
}
    
void TextureDescriptor::SaveAsBinary(const String &filePathname)
{
    File *file = File::Create(filePathname, File::WRITE | File::OPEN | File::CREATE);
    if(!file)
    {
        Logger::Error("[TextureDescriptor::Save] Can't open file: %s", filePathname.c_str());
        return;
    }

    WriteSignature(file, BINARY_SIGNATURE);
    
    file->Write(&wrapModeS, sizeof(wrapModeS));
    file->Write(&wrapModeT, sizeof(wrapModeT));
    file->Write(&generateMipMaps, sizeof(generateMipMaps));
    file->Write(&baseMipMapLevel, sizeof(baseMipMapLevel));
    
    //TODO: Write used fileExtension/Format
    
    SafeRelease(file);
}

    
    
TextureDescriptor::eFileType TextureDescriptor::DetectFileType(DAVA::File *file)
{
    char8 lineData[LINE_SIZE];
    uint32 lineSize = file->ReadLine(lineData, LINE_SIZE);
    if(lineSize)
    {
        int32 signature = lineData[0] | (lineData[1] << 8) | (lineData[2] << 16) | (lineData[3] << 24);
        if(BINARY_SIGNATURE == signature)
        {
            return TYPE_BINARY;
        }
    }

    return TYPE_TEXT;
}
    
    
void TextureDescriptor::LoadAsText(File *file)
{
    ReadChar8String(file, modificationDate, DATE_BUFFER_SIZE);
    
    char8 readableCrc[MD5::DIGEST_SIZE*2 + 1];
    ReadChar8String(file, readableCrc, MD5::DIGEST_SIZE*2 + 1);
    CrcFromReadableFormat(readableCrc);
    
    ReadInt8(file, wrapModeS);
    ReadInt8(file, wrapModeT);
    ReadInt8(file, generateMipMaps);
    ReadInt8(file, baseMipMapLevel);
    
    ReadCompressionAsText(file, pvrCompression);
    ReadCompressionAsText(file, dxtCompression);
}
    
void TextureDescriptor::LoadAsBinary(File *file)
{
    file->Read(&wrapModeS, sizeof(wrapModeS));
    file->Read(&wrapModeT, sizeof(wrapModeT));
    file->Read(&generateMipMaps, sizeof(generateMipMaps));
    file->Read(&baseMipMapLevel, sizeof(baseMipMapLevel));
    
    //TODO: Read used fileExtension/Format
}

void TextureDescriptor::WriteSignature(File *file, int32 signature)
{
    file->Write(&signature, sizeof(int32));
}

    
void TextureDescriptor::ReadCompressionAsText(File *file, Compression &compression)
{
    char8 formatName[DATE_BUFFER_SIZE];
    ReadChar8String(file, formatName, DATE_BUFFER_SIZE);
    compression.format = Texture::GetPixelFormatByName(String(formatName));
    
    ReadInt8(file, compression.flipVertically);
}

void TextureDescriptor::ReadCompressionAsBinary(File *file, Compression &compression)
{
    int32 format = FORMAT_INVALID;
    file->Read(&format, sizeof(int32));
    compression.format = (PixelFormat)format;
    
    file->Read(&compression.flipVertically, sizeof(int32));
}

void TextureDescriptor::WriteCompressionAsText(File *file, const Compression &compression)
{
    WriteChar8String(file, Texture::GetPixelFormatString(compression.format));
    WriteInt8(file, compression.flipVertically);
}

void TextureDescriptor::WriteCompressionAsBinary(File *file, const Compression &compression)
{
    int32 format = compression.format;
    file->Write(&format, sizeof(int32));
    file->Write(&compression.flipVertically, sizeof(int32));
}
    
void TextureDescriptor::ReadInt8(DAVA::File *file, int8 &value)
{
    char8 lineData[LINE_SIZE];
    uint32 lineSize = file->ReadLine(lineData, LINE_SIZE);
    if(lineSize)
    {
        int32 readValue = 0;
        sscanf(lineData, "%d", &readValue);
        value = readValue;
    }
}
    
void TextureDescriptor::WriteInt8(DAVA::File *file, const int8 value)
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
        Snprinf(buffer, bufferSize, "%s", lineData);
        buffer[bufferSize-1] = 0;
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
    
bool TextureDescriptor::GenerateMipMaps()
{
    return (OPTION_DISABLED != generateMipMaps);
}
    

String TextureDescriptor::GetDefaultExtension()
{
    return String(".tde");
}

    
};
