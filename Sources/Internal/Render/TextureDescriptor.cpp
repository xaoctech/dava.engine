
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
#include "FileSystem/FileSystem.h"
#include "FileSystem/DynamicMemoryFile.h"
#include "Render/Texture.h"
#include "Utils/Utils.h"

namespace DAVA
{
    
void TextureDescriptor::Compression::Clear()
{
    format = FORMAT_INVALID;
    Memset(modificationDate, 0, DATE_BUFFER_SIZE * sizeof(char8));
    Memset(crc, 0, MD5::DIGEST_SIZE * sizeof(uint8));

    compressToWidth = 0;
    compressToHeight = 0;
}
    
    
TextureDescriptor::TextureDescriptor()
{
    isCompressedFile = false;
    InitializeValues();
}

TextureDescriptor::~TextureDescriptor()
{
#if defined TEXTURE_SPLICING_ENABLED
    SafeRelease(textureFile);
#endif //#if defined TEXTURE_SPLICING_ENABLED

}

TextureDescriptor *TextureDescriptor::CreateFromFile(const String &filePathname)
{
    String descriptorPathname = GetDescriptorPathname(filePathname);
    TextureDescriptor *descriptor = new TextureDescriptor();
    bool loaded = descriptor->Load(descriptorPathname);
    if(!loaded)
    {
        Logger::Error("[TextureDescriptor::CreateFromFile(]: there are no descriptor file (%s).", descriptorPathname.c_str());
        SafeRelease(descriptor);
        return NULL;
    }
    
    return descriptor;
}
    
    
void TextureDescriptor::InitializeValues()
{
    SetDefaultValues();
    
    pvrCompression.Clear();
    dxtCompression.Clear();
    
#if defined TEXTURE_SPLICING_ENABLED
    textureFile = NULL;
#endif //#if defined TEXTURE_SPLICING_ENABLED

    textureFileFormat = PNG_FILE;
}
    
void TextureDescriptor::SetDefaultValues()
{
    wrapModeS = Texture::WRAP_REPEAT;
    wrapModeT = Texture::WRAP_REPEAT;
    
    generateMipMaps = OPTION_ENABLED;
    
    minFilter = Texture::FILTER_LINEAR_MIPMAP_LINEAR;
    magFilter = Texture::FILTER_LINEAR;
}
    
bool TextureDescriptor::UpdateDateAndCrcForFormat(ImageFileFormat fileFormat) const
{
    bool wasUpdated = false;
    
    const Compression *compression = GetCompressionParams(fileFormat);
    
	String filePathname = GetSourceTexturePathname();
    String date = File::GetModificationDate(filePathname);
    if(date.empty())
    {
        Memset(compression->modificationDate, 0, DATE_BUFFER_SIZE * sizeof(char8));

        if(compression->crc[0] != 0)
        {
            Memset(compression->crc, 0, MD5::DIGEST_SIZE * sizeof(uint8));
            wasUpdated = true;
        }
    }
    else
    {
        strncpy(compression->modificationDate, date.c_str(), DATE_BUFFER_SIZE-1);
        compression->modificationDate[DATE_BUFFER_SIZE-1] = 0;
        
        uint8 crc[MD5::DIGEST_SIZE];
        MD5::ForFile(filePathname, crc);
        
        int32 cmpResult = Memcmp(crc, compression->crc, MD5::DIGEST_SIZE * sizeof(uint8));
        if(0 != cmpResult)
        {
            Memcpy(compression->crc, crc, MD5::DIGEST_SIZE * sizeof(uint8));
            wasUpdated = true;
        }
    }
    
    return wasUpdated;
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

    int8 version = 0;
    file->Read(&version, sizeof(version));
    if(version != CURRENT_VERSION)
    {
        ConvertToCurrentVersion(version, signature, file);
        SafeRelease(file);
        return true;
    }
    
    isCompressedFile = (COMPRESSED_FILE == signature);
    if(isCompressedFile)
    {
        LoadCompressed(file);
    }
    else if(NOTCOMPRESSED_FILE == signature)
    {
        LoadNotCompressed(file);
    }
    else
    {
        Logger::Error("[TextureDescriptor::Load] Wrong descriptor file: %s", filePathname.c_str());
        SafeRelease(file);
        return false;
    }
    
    SafeRelease(file);
    
    return true;
}

void TextureDescriptor::Save() const
{
    DVASSERT_MSG(!pathname.empty(), "Can use this method only after calling Load()");
    Save(pathname);
}
    
void TextureDescriptor::Save(const String &filePathname) const
{
    File *file = File::Create(filePathname, File::WRITE | File::OPEN | File::CREATE);
    if(!file)
    {
        Logger::Error("[TextureDescriptor::Save] Can't open file: %s", filePathname.c_str());
        return;
    }
    
    int32 signature = NOTCOMPRESSED_FILE;
    file->Write(&signature, sizeof(signature));
    
    int8 version = CURRENT_VERSION;
    file->Write(&version, sizeof(version));
    
    WriteGeneralSettings(file);
    
    //Compression
    WriteCompression(file, pvrCompression);
    WriteCompression(file, dxtCompression);
    
    SafeRelease(file);
}
  
#if defined TEXTURE_SPLICING_ENABLED
void TextureDescriptor::ExportAndSplice(const String &filePathname, const String &texturePathname)
{
    File *file = File::Create(filePathname, File::WRITE | File::OPEN | File::CREATE);
    if(!file)
    {
        Logger::Error("[TextureDescriptor::ExportAndSplice] Can't open file: %s", filePathname.c_str());
        return;
    }
    
    int32 signature = COMPRESSED_FILE;
    file->Write(&signature, sizeof(signature));
    
    int8 version = CURRENT_VERSION;
    file->Write(&version, sizeof(version));

    WriteGeneralSettings(file);
    file->Write(&textureFileFormat, sizeof(textureFileFormat));
    
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
    
    SafeRelease(file);
}

#else //#if defined TEXTURE_SPLICING_ENABLED
void TextureDescriptor::Export(const String &filePathname)
{
    File *file = File::Create(filePathname, File::WRITE | File::OPEN | File::CREATE);
    if(!file)
    {
        Logger::Error("[TextureDescriptor::ExportAndSplice] Can't open file: %s", filePathname.c_str());
        return;
    }

    int32 signature = COMPRESSED_FILE;
    file->Write(&signature, sizeof(signature));
    
    int8 version = CURRENT_VERSION;
    file->Write(&version, sizeof(version));

    WriteGeneralSettings(file);
    file->Write(&textureFileFormat, sizeof(textureFileFormat));

    SafeRelease(file);
}
    
#endif //#if defined TEXTURE_SPLICING_ENABLED


void TextureDescriptor::ConvertToCurrentVersion(int8 version, int32 signature, DAVA::File *file)
{
//    Logger::Info("[TextureDescriptor::ConvertToCurrentVersion] (%s) from version %d", pathname.c_str(), version);
    
    if(version == 2)
    {
        LoadVersion2(signature, file);
    }
	if(version == 3)
	{
		LoadVersion3(signature, file);
	}
}
    
void TextureDescriptor::LoadVersion2(int32 signature, DAVA::File *file)
{
    if(signature == COMPRESSED_FILE)
    {
#if defined TEXTURE_SPLICING_ENABLED
        SafeRelease(textureFile);
#endif //#if defined TEXTURE_SPLICING_ENABLED
        
        file->Read(&wrapModeS, sizeof(wrapModeS));
        file->Read(&wrapModeT, sizeof(wrapModeT));
        file->Read(&generateMipMaps, sizeof(generateMipMaps));
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
    else if(signature == NOTCOMPRESSED_FILE)
    {
        file->Read(&wrapModeS, sizeof(wrapModeS));
        file->Read(&wrapModeT, sizeof(wrapModeT));
        file->Read(&generateMipMaps, sizeof(generateMipMaps));
        
        ReadCompressionWithDateOld(file, pvrCompression);
        ReadCompressionWithDateOld(file, dxtCompression);
    }
}
 
void TextureDescriptor::LoadVersion3(int32 signature, DAVA::File *file)
{
	if(signature == COMPRESSED_FILE)
	{
#if defined TEXTURE_SPLICING_ENABLED
		SafeRelease(textureFile);
#endif //#if defined TEXTURE_SPLICING_ENABLED

		file->Read(&wrapModeS, sizeof(wrapModeS));
		file->Read(&wrapModeT, sizeof(wrapModeT));
		file->Read(&generateMipMaps, sizeof(generateMipMaps));
		file->Read(&minFilter, sizeof(minFilter));
		file->Read(&magFilter, sizeof(magFilter));
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
	else if(signature == NOTCOMPRESSED_FILE)
	{
		file->Read(&wrapModeS, sizeof(wrapModeS));
		file->Read(&wrapModeT, sizeof(wrapModeT));
		file->Read(&generateMipMaps, sizeof(generateMipMaps));
		file->Read(&minFilter, sizeof(minFilter));
		file->Read(&magFilter, sizeof(magFilter));

		ReadCompressionWithDateOld(file, pvrCompression);
		ReadCompressionWithDateOld(file, dxtCompression);
	}
}


void TextureDescriptor::LoadNotCompressed(File *file)
{
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
    file->Read(&minFilter, sizeof(minFilter));
    file->Read(&magFilter, sizeof(magFilter));
}
    
void TextureDescriptor::WriteGeneralSettings(File *file) const
{
    file->Write(&wrapModeS, sizeof(wrapModeS));
    file->Write(&wrapModeT, sizeof(wrapModeT));
    file->Write(&generateMipMaps, sizeof(generateMipMaps));
    file->Write(&minFilter, sizeof(minFilter));
    file->Write(&magFilter, sizeof(magFilter));
}

    
void TextureDescriptor::ReadCompression(File *file, Compression &compression)
{
    int8 format;
    file->Read(&format, sizeof(format));
    compression.format = (PixelFormat)format;

    file->Read(&compression.compressToWidth, sizeof(compression.compressToWidth));
    file->Read(&compression.compressToHeight, sizeof(compression.compressToHeight));

    char8 crcString[MD5::DIGEST_SIZE*2 + 1];
    file->Read(crcString, (MD5::DIGEST_SIZE*2 + 1) * sizeof(char8));
    MD5::CharToHash(crcString, compression.crc);

	Memset(compression.modificationDate, 0, DATE_BUFFER_SIZE * sizeof(char8));
	if(crcString[0])
	{
		String filePathname = GetSourceTexturePathname();
		String date = File::GetModificationDate(filePathname);
		if(!date.empty())
		{
			strncpy(compression.modificationDate, date.c_str(), DATE_BUFFER_SIZE-1);
			compression.modificationDate[DATE_BUFFER_SIZE-1] = 0;
		}
	}
}

void TextureDescriptor::ReadCompressionWithDateOld( File *file, Compression &compression )
{
	int8 format;
	file->Read(&format, sizeof(format));
	compression.format = (PixelFormat)format;

	file->Read(&compression.compressToWidth, sizeof(compression.compressToWidth));
	file->Read(&compression.compressToHeight, sizeof(compression.compressToHeight));

 	file->Read(compression.modificationDate, DATE_BUFFER_SIZE * sizeof(char8));

	char8 crcString[MD5::DIGEST_SIZE*2 + 1];
	file->Read(crcString, (MD5::DIGEST_SIZE*2 + 1) * sizeof(char8));
	MD5::CharToHash(crcString, compression.crc);
}

void TextureDescriptor::WriteCompression(File *file, const Compression &compression) const
{
    int8 format = compression.format;
    file->Write(&format, sizeof(format));
    file->Write(&compression.compressToWidth, sizeof(compression.compressToWidth));
    file->Write(&compression.compressToHeight, sizeof(compression.compressToHeight));
    
    //crc
    char8 crcString[MD5::DIGEST_SIZE*2 + 1];
    MD5::HashToChar(compression.crc, crcString, MD5::DIGEST_SIZE*2 + 1);
    file->Write(crcString, (MD5::DIGEST_SIZE*2 + 1) * sizeof(char8));
}


bool TextureDescriptor::GetGenerateMipMaps() const
{
    return (OPTION_DISABLED != generateMipMaps);
}
    
    
String TextureDescriptor::GetSourceTexturePathname() const
{
    if(pathname.empty())
    {
        return String("");
    }
    
    return FileSystem::Instance()->ReplaceExtension(pathname, GetSourceTextureExtension());
}

String TextureDescriptor::GetDescriptorPathname(const String &texturePathname)
{
    return FileSystem::Instance()->ReplaceExtension(texturePathname, GetDescriptorExtension());
}


String TextureDescriptor::GetDescriptorExtension()
{
    return String(".tex");
}
    
String TextureDescriptor::GetSourceTextureExtension()
{
    return String(".png");
}
    
bool TextureDescriptor::IsSourceValidForFormat(ImageFileFormat fileFormat)
{
    const Compression *compression = GetCompressionParams(fileFormat);

    const String sourceTexturePathname = GetSourceTexturePathname();
    String modificationDate = File::GetModificationDate(sourceTexturePathname);
    
    if(!modificationDate.empty() && (0 != CompareCaseInsensitive(modificationDate, String(compression->modificationDate))))
    {
        uint8 crc[MD5::DIGEST_SIZE];
        MD5::ForFile(sourceTexturePathname, crc);
        
        int32 cmpResult = Memcmp(crc, compression->crc, MD5::DIGEST_SIZE * sizeof(uint8));
        return (0 != cmpResult);
    }

    return false;
}

    
const TextureDescriptor::Compression * TextureDescriptor::GetCompressionParams(ImageFileFormat fileFormat) const
{
    DVASSERT((fileFormat == PVR_FILE) || (fileFormat == DXT_FILE));
    
    if(fileFormat == PVR_FILE)
    {
        return &pvrCompression;
    }
    else if(fileFormat == DXT_FILE)
    {
        return &dxtCompression;
    }
    
    return NULL;
}

String TextureDescriptor::GetSupportedTextureExtensions()
{
    return String(".png;.pvr;") + TextureDescriptor::GetDescriptorExtension();
}

String TextureDescriptor::GetPathnameForFormat(const String &pathname, ImageFileFormat fileFormat)
{
    if(fileFormat == PNG_FILE)
    {
        return FileSystem::Instance()->ReplaceExtension(pathname, ".png");
    }
    else if(fileFormat == PVR_FILE)
    {
        return FileSystem::Instance()->ReplaceExtension(pathname, ".pvr");
    }
    else if(fileFormat == DXT_FILE)
    {
        return FileSystem::Instance()->ReplaceExtension(pathname, ".dds");
    }

    return String("");
}
    
ImageFileFormat TextureDescriptor::GetFormatForPathname(const String &pathname)
{
    String extension = FileSystem::GetExtension(pathname);
    return GetFormatForExtension(extension);
}
    

ImageFileFormat TextureDescriptor::GetFormatForExtension(const String &extension)
{
    if(0 == CompareCaseInsensitive(extension, ".png"))
    {
        return PNG_FILE;
    }
    else if(0 == CompareCaseInsensitive(extension, ".pvr"))
    {
        return PVR_FILE;
    }
    else if(0 == CompareCaseInsensitive(extension, ".dds"))
    {
        return DXT_FILE;
    }

    return NOT_FILE;
}

const bool TextureDescriptor::IsCompressedFile() const
{
    return isCompressedFile;
}

};
