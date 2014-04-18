/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#include "Render/ImageLoader.h"
#include "FileSystem/File.h"
#include "FileSystem/FileSystem.h"
#include "Render/RenderBase.h"

#include "Platform/SystemTimer.h"
#include "Utils/Utils.h"
#include "Render/ImageConvert.h"

namespace DAVA 
{
    
bool ImageLoader::CreateFromFileByExtension(const FilePath &pathname, Vector<Image *> & imageSet, int32 baseMipmap /*= 0*/)
{
    if(pathname.IsEqualToExtension("*.pvr"))
    {
        return ImageLoader::CreateFromPVRFile(pathname, imageSet, baseMipmap);
    }
    else if(pathname.IsEqualToExtension("*.dds"))
    {
        return ImageLoader::CreateFromDDSFile(pathname, imageSet, baseMipmap);
    }
    
    return ImageLoader::CreateFromFileByContent(pathname, imageSet, baseMipmap);
}

    
    
bool ImageLoader::CreateFromFileByContent(const FilePath & pathname, Vector<Image *> & imageSet, int32 baseMipmap /*= 0*/)
{
    File *file = File::Create(pathname, File::OPEN | File::READ);
    
    if(!file)
    {
        Logger::Error("[ImageLoader::CreateFromFile] Cannot open file %s", pathname.GetAbsolutePathname().c_str());
        return false;
    }
    
    bool created = CreateFromFileByContent(file, imageSet, baseMipmap);
    SafeRelease(file);
	return created;
};
    
    
bool ImageLoader::CreateFromFileByContent(File *file, Vector<Image *> & imageSet, int32 baseMipmap /*= 0*/)
{
    if(IsPVRFile(file))
    {
        return CreateFromPVR(file, imageSet, baseMipmap);
    }

    if(IsPNGFile(file))
    {
        return CreateFromPNG(file, imageSet);
    }

    if(IsDDSFile(file))
    {
        return CreateFromDDS(file, imageSet, baseMipmap);
    }
    
    if(IsJPEGFile(file))
    {
        return CreateFromJPEG(file, imageSet);
    }
    
    return false;
}

    
bool ImageLoader::CreateFromPNGFile(const FilePath & pathname, Vector<Image *> & imageSet)
{
    File *file = File::Create(pathname, File::OPEN | File::READ);
    
    if(!file)
    {
        Logger::Error("[ImageLoader::CreateFromPNGFile] Cannot open file %s", pathname.GetAbsolutePathname().c_str());
        return false;
    }
    
    bool created = CreateFromPNG(file, imageSet);
    SafeRelease(file);
	return created;
}
    
bool ImageLoader::CreateFromJPEGFile(const FilePath & pathname, Vector<Image *> & imageSet)
{
  /*  Image *jpegImage = new Image();
    if(jpegImage)
    {
        bool created = LibJpegWrapper::ReadJpegFile(pathname, jpegImage);
        if(created)
        {
            imageSet.push_back(jpegImage);
            return true;
        }
        
        SafeRelease(jpegImage);
    }
    */
    return false;
}
    
bool ImageLoader::CreateFromPVRFile(const FilePath & pathname, Vector<Image *> & imageSet, int32 baseMipmap /*= 0*/)
{
    File *file = File::Create(pathname, File::OPEN | File::READ);
    
    if(!file)
    {
        Logger::Error("[ImageLoader::CreateFromPVRFile] Cannot open file %s", pathname.GetAbsolutePathname().c_str());
        return false;
    }
    
    bool created = CreateFromPVR(file, imageSet, baseMipmap);
    SafeRelease(file);
	return created;
}
    
bool ImageLoader::CreateFromDDSFile(const FilePath & pathname, Vector<Image *> & imageSet, int32 baseMipmap /*= 0*/)
{
    File *file = File::Create(pathname, File::OPEN | File::READ);
    
    if(!file)
    {
        Logger::Error("[ImageLoader::CreateFromDDSFile] Cannot open file %s", pathname.GetAbsolutePathname().c_str());
        return false;
    }
    
    bool created = CreateFromDDS(file, imageSet, baseMipmap);
    SafeRelease(file);
	return created;
}


bool ImageLoader::IsPNGFile(DAVA::File *file)
{
   /* bool isPng = LibPngWrapper::IsPngFile(file);
    file->Seek(0, File::SEEK_FROM_START);
    return isPng;*/
    return 0;
}
    
bool ImageLoader::IsJPEGFile(File *file)
{
  /*  bool isJpeg = LibJpegWrapper::IsJpegFile(file->GetFilename());
    file->Seek(0, File::SEEK_FROM_START);
    return isJpeg;*/
    return 0;
}
    
bool ImageLoader::IsPVRFile(DAVA::File *file)
{
    /*bool isPvr = LibPVRHelper::IsPvrFile(file);
    file->Seek(0, File::SEEK_FROM_START);
    return isPvr;*/
    return 0;
}

bool ImageLoader::IsDDSFile(DAVA::File *file)
{
   /* bool isDXT = LibDdsHelper::IsDxtFile(file);
    file->Seek(0, File::SEEK_FROM_START);
    return isDXT;*/
    return 0;
}
    
    
bool ImageLoader::CreateFromPNG(DAVA::File *file, Vector<Image *> & imageSet)
{
  /*  Image *pngImage = new Image();
    if(pngImage)
    {
        int32 retCode = LibPngWrapper::ReadPngFile(file, pngImage);
        if(1 == retCode)
        {
            imageSet.push_back(pngImage);
            return true;
        }
        
        SafeRelease(pngImage);
    }*/
    
    return false;
}
    
bool ImageLoader::CreateFromJPEG(File *file, Vector<Image *> & imageSet)
{
    return CreateFromJPEGFile(file->GetFilename(), imageSet);
}

bool ImageLoader::CreateFromDDS(DAVA::File *file, Vector<Image *> & imageSet, int32 baseMipmap /*= 0*/)
{
	//return LibDdsHelper::ReadDxtFile(file, imageSet, baseMipmap);
}

bool ImageLoader::CreateFromPVR(DAVA::File *file, Vector<Image *> & imageSet, int32 baseMipmap /*= 0*/)
{
//    uint64 loadTime = SystemTimer::Instance()->AbsoluteMS();

  /*  int32 mipMapLevelsCount = LibPVRHelper::GetMipMapLevelsCount(file);
    baseMipmap = Min(baseMipmap, mipMapLevelsCount - 1);

	int32 faceCount = LibPVRHelper::GetCubemapFaceCount(file);
	int32 totalImageCount = (mipMapLevelsCount - baseMipmap) * faceCount;
    if(totalImageCount)
    {
        imageSet.reserve(totalImageCount);
        for(int32 i = 0; i < totalImageCount; ++i)
        {
            Image *image = new Image();
            imageSet.push_back(image);
        }

        file->Seek(0, File::SEEK_FROM_START);
        bool read = LibPVRHelper::ReadFile(file, imageSet, baseMipmap);
        if(!read)
        {
            Logger::Error("[ImageLoader::CreateFromPVR] Cannot read images from PVR file (%s)", file->GetFilename().GetAbsolutePathname().c_str());
			for_each(imageSet.begin(), imageSet.end(), SafeRelease<Image>);
            return false;
        }
//        loadTime = SystemTimer::Instance()->AbsoluteMS() - loadTime;
//        Logger::Info("Unpack PVR(%s) for %ldms", file->GetFilename().c_str(), loadTime);
        return true;
    }*/
    return false;
}

bool ImageLoader::Save(const DAVA::Image *image, const FilePath &pathname)
{
    bool retValue = false;
    /*DVASSERT(image && (pathname.IsEqualToExtension(".png") || pathname.IsEqualToExtension(".jpg") || pathname.IsEqualToExtension(".jpeg")));
    
    DVASSERT((FORMAT_RGB888 == image->format)||(FORMAT_RGBA8888 == image->format) ||
             (FORMAT_A8 == image->format) || (FORMAT_A16 == image->format));
    
    Image* imgToSave = NULL;
    ImageFileWrapperFactory::eSupportedImageFileFormats targetImageFormat;
    
    if(pathname.IsEqualToExtension(".png"))
    {
        targetImageFormat = ImageFileWrapperFactory::FILE_FORMAT_PNG;
        if(FORMAT_RGB888 == image->format)
        {
            Image* imgToSave = Image::Create(image->width, image->height, FORMAT_RGBA8888);
            
            ConvertDirect<RGB888, uint32, ConvertRGB888toRGBA8888> convert;
            convert(image->data, image->width, image->height, sizeof(RGB888)*image->width, imgToSave->data, imgToSave->width, imgToSave->height, sizeof(uint32)*image->width);
        }
    }
    else if(pathname.IsEqualToExtension(".jpg") || pathname.IsEqualToExtension(".jpeg"))
    {
        targetImageFormat = ImageFileWrapperFactory::FILE_FORMAT_JPEG;
        if(!(FORMAT_RGB888 == image->format) || (FORMAT_A8 == image->format))
        {
            if(FORMAT_RGBA8888 == image->format)
            {
                imgToSave = Image::Create(image->width, image->height, FORMAT_RGB888);
                ConvertDirect<uint32, RGB888, ConvertRGBA8888toRGB888> convert;
                convert(image->data, image->width, image->height, sizeof(uint32)*image->width, imgToSave->data, imgToSave->width, imgToSave->height, sizeof(RGB888)*image->width);
            }
            else if(FORMAT_A16 == image->format)
            {
                imgToSave = Image::Create(image->width, image->height, FORMAT_A8);
                ConvertDirect<uint16, uint8, ConvertA16toA8> convert;
                convert(image->data, image->width, image->height, sizeof(uint16)*image->width, imgToSave->data, imgToSave->width, imgToSave->height, sizeof(uint8)*image->width);
            }
        }
    }
    
    if(imgToSave != NULL)
    {
        retValue = ImageFileWrapperFactory::GetImageFileWrapper(targetImageFormat)->WriteFile(pathname, imgToSave->width, imgToSave->height, imgToSave->data, imgToSave->format);
        SafeRelease(imgToSave);
    }
    else
    {
        retValue = ImageFileWrapperFactory::GetImageFileWrapper(targetImageFormat)->WriteFile(pathname, image->width, image->height, image->data, image->format);
    }*/
    return retValue;
}
 /*
ImageFileWapper* ImageFileWrapperFactory::GetImageFileWrapper(eSupportedImageFileFormats format)
{
    switch (format)
    {
        case FILE_FORMAT_PNG:
            return &pngHelper;
            break;
        case FILE_FORMAT_DDS:
            return &dxtHelper;
            break;
        case FILE_FORMAT_PVR:
            return &pvrHelper;
            break;
        case FILE_FORMAT_JPEG:
            return &jpegHelper;
            break;
    }
    
    DVASSERT(0);
    return NULL;
}

ImageFileWapper* ImageFileWrapperFactory::GetImageFileWrapper(const FilePath&)
{
    DVASSERT(0);
    return NULL;
}
    */
};
