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
    static const int32 DATE_BUFFER_SIZE = 20;
    static const int32 LINE_SIZE = 256;
    static const int8 CURRENT_VERSION = 4;
    
public:
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
    
    struct Compression
    {
        PixelFormat format;
        mutable char8 modificationDate[DATE_BUFFER_SIZE];
        mutable uint8 crc[MD5::DIGEST_SIZE];
        
        int32 compressToWidth;
        int32 compressToHeight;
        
        void Clear();
    };

public:
    TextureDescriptor();
    virtual ~TextureDescriptor();
    
    static TextureDescriptor *CreateFromFile(const String &filePathname);
    
    
    bool UpdateDateAndCrcForFormat(ImageFileFormat fileFormat) const;

    bool Load(const String &filePathname);

    void Save() const;
    void Save(const String &filePathname) const;
    
#if defined TEXTURE_SPLICING_ENABLED
    void ExportAndSplice(const String &filePathname, const String &texturePathname);
#else //#if defined TEXTURE_SPLICING_ENABLED
    void Export(const String &filePathname);
#endif //#if defined TEXTURE_SPLICING_ENABLED

    bool GetGenerateMipMaps() const;

    String GetSourceTexturePathname() const; 
    static String GetSourceTextureExtension(); 

    static String GetSupportedTextureExtensions();

    static String GetDescriptorPathname(const String &texturePathname);
    static String GetDescriptorExtension();
    
    bool IsSourceValidForFormat(ImageFileFormat fileFormat);
    
    static String GetPathnameForFormat(const String &pathname, ImageFileFormat fileFormat);
    static ImageFileFormat GetFormatForPathname(const String &pathname);
    static ImageFileFormat GetFormatForExtension(const String &extension);

    void SetDefaultValues();
    
    const bool IsCompressedFile() const;
    
protected:
    
    const Compression * GetCompressionParams(ImageFileFormat fileFormat) const;
    
    void LoadNotCompressed(File *file);
    void LoadCompressed(File *file);
    
    void InitializeValues();
    
    void ReadGeneralSettings(File *file);
    void WriteGeneralSettings(File *file) const;
    
    void ReadCompression(File *file, Compression &compression);
	void ReadCompressionWithDateOld(File *file, Compression &compression);
    void WriteCompression(File *file, const Compression &compression) const;
    
    
    void ConvertToCurrentVersion(int8 version, int32 signature, File *file);
    void LoadVersion2(int32 signature, File *file);
	void LoadVersion3(int32 signature, File *file);
    
public:
    
    int8 wrapModeS;
    int8 wrapModeT;
    
    int8 generateMipMaps;
    
    int8 minFilter;
    int8 magFilter;
    
    
    Compression pvrCompression;
    Compression dxtCompression;
    
    //Binary only
    int8 textureFileFormat;
#if defined TEXTURE_SPLICING_ENABLED
    File *textureFile;
#endif //#if defined TEXTURE_SPLICING_ENABLED
    
    String pathname;
    
    bool isCompressedFile;
};
    
};
#endif // __DAVAENGINE_TEXTURE_DESCRIPTOR_H__
