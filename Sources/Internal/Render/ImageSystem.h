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


#ifndef __DAVAENGINE_IMAGE_SYSTEM_H__
#define __DAVAENGINE_IMAGE_SYSTEM_H__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "FileSystem/FilePath.h"
#include "Render/ImageFormatInterface.h"
#include "Render/LibJpegHelper.h"
#include "Render/LibDdsHelper.h"
#include "Render/LibPngHelpers.h"
#include "Render/LibPVRHelper.h"
#include "FileSystem/File.h"


namespace DAVA 
{
class Image;

    
class ImageSystem: public Singleton<ImageSystem>
{
public:
    enum eSupportedImageFileFormats
    {
        FILE_FORMAT_PNG = 0,
        FILE_FORMAT_DDS,
        FILE_FORMAT_PVR,
        FILE_FORMAT_JPEG,
        FILE_FORMAT_COUNT
    };
    
    ImageSystem();
    
    virtual ~ImageSystem()
    {
        for(size_t i = 0; i < FILE_FORMAT_COUNT; ++i)
        {
            delete wrappers[i];
        }
    }
    
    ImageFormatInterface* GetImageFormatInterface(eSupportedImageFileFormats fileFormat)
    {
        return wrappers[fileFormat];
    }
    
    eErrorCode Load(const FilePath & pathname, Vector<Image *> & imageSet, int32 baseMipmap = 0)
    {
        File *fileRead = File::Create(pathname, File::READ | File::OPEN);
        if(!fileRead)
        {
            return ERROR_FILE_NOT_FOUND;
        }
        
        eErrorCode result = Load(fileRead, imageSet, baseMipmap);
        
        SafeRelease(fileRead);
        
        return result;
    }
    
    eErrorCode Load(File *file, Vector<Image *> & imageSet, int32 baseMipmap = 0)
    {
        file->Seek(0,  File::SEEK_FROM_START);
        ImageFormatInterface* propperWrapper = DetectImageFormatInterfaceByExtension(file->GetFilename());
        
        if (NULL == propperWrapper || !propperWrapper->IsImage(file))
        {
            return ERROR_FILE_FORMAT_INCORRECT;
        }
        
        return propperWrapper->ReadFile(file, imageSet, baseMipmap) ? SUCCESS : ERROR_READ_FAIL;
    }
    
    eErrorCode Save(const FilePath & fileName, const Vector<Image *> &imageSet, PixelFormat compressionFormat, bool isCubeMap = false)
    {
        ImageFormatInterface* propperWrapper = DetectImageFormatInterfaceByExtension(fileName);
        if(!propperWrapper)
        {
            return ERROR_FILE_FORMAT_INCORRECT;
        }

        return propperWrapper->WriteFile(fileName, imageSet, compressionFormat, isCubeMap);
    }
    
protected:
    
    ImageFormatInterface* DetectImageFormatInterfaceByExtension(const FilePath & pathname)
    {
        if(pathname.IsEqualToExtension(".pvr"))
        {
            return wrappers[FILE_FORMAT_PVR];
        }
        else if(pathname.IsEqualToExtension(".dds"))
        {
            return wrappers[FILE_FORMAT_DDS];
        }
        else if(pathname.IsEqualToExtension(".png"))
        {
            return wrappers[FILE_FORMAT_PNG];
        }
        else if(pathname.IsEqualToExtension(".jpeg")||pathname.IsEqualToExtension(".jpg"))
        {
            return wrappers[FILE_FORMAT_JPEG];
        }
        DVASSERT(0);
        
        return NULL;
    }
    
    ImageFormatInterface* DetectImageFormatInterfaceByContent(File *file)
    {
        if( wrappers[FILE_FORMAT_PVR]->IsImage(file))
        {
            return  wrappers[FILE_FORMAT_PVR];
        }
        else if(wrappers[FILE_FORMAT_DDS]->IsImage(file))
        {
            return wrappers[FILE_FORMAT_DDS];
        }
        else if(wrappers[FILE_FORMAT_PNG]->IsImage(file))
        {
            return wrappers[FILE_FORMAT_PNG];
        }
        else if(wrappers[FILE_FORMAT_JPEG]->IsImage(file))
        {
            return wrappers[FILE_FORMAT_JPEG];
        }
        DVASSERT(0);
        
        return NULL;
    }
       
    ImageFormatInterface* wrappers[FILE_FORMAT_COUNT];
};
   
};

#endif // __DAVAENGINE_IMAGE_SYSTEM_H__
