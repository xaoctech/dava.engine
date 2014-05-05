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


#include "Render/ImageSystem.h"
#include "FileSystem/File.h"
#include "FileSystem/FileSystem.h"
#include "Render/RenderBase.h"

#include "Platform/SystemTimer.h"
#include "Utils/Utils.h"

#include "Render/LibJpegHelper.h"
#include "Render/LibDdsHelper.h"
#include "Render/LibPngHelpers.h"
#include "Render/LibPVRHelper.h"

namespace DAVA 
{

ImageSystem::ImageSystem()
{
    wrappers[FILE_FORMAT_PNG] = new LibPngWrapper();
    wrappers[FILE_FORMAT_DDS] = new LibDdsHelper();
    wrappers[FILE_FORMAT_PVR] = new LibPVRHelper();
    wrappers[FILE_FORMAT_JPEG] = new LibJpegWrapper();
}
    
ImageSystem::~ImageSystem()
{
    for(size_t i = 0; i < FILE_FORMAT_COUNT; ++i)
    {
        delete wrappers[i];
    }
}

eErrorCode ImageSystem::Load(const FilePath & pathname, Vector<Image *> & imageSet, int32 baseMipmap)
{
    File *fileRead = File::Create(pathname, File::READ | File::OPEN);
    if(!fileRead)
    {
        return ERROR_FILE_NOTFOUND;
    }
    
    eErrorCode result = Load(fileRead, imageSet, baseMipmap);
    
    SafeRelease(fileRead);
    
    return result;
}

eErrorCode ImageSystem::Load(File *file, Vector<Image *> & imageSet, int32 baseMipmap)
{
    ImageFormatInterface* propperWrapper = DetectImageFormatInterfaceByExtension(file->GetFilename());
    
    if (NULL == propperWrapper || !propperWrapper->IsImage(file))
    {
        return ERROR_FILE_FORMAT_INCORRECT;
    }
    file->Seek(0,  File::SEEK_FROM_START);
    return propperWrapper->ReadFile(file, imageSet, baseMipmap);
}

eErrorCode ImageSystem::Save(const FilePath & fileName, const Vector<Image *> &imageSet, PixelFormat compressionFormat, bool isCubeMap)
{
    ImageFormatInterface* propperWrapper = DetectImageFormatInterfaceByExtension(fileName);
    if(!propperWrapper)
    {
        return ERROR_FILE_FORMAT_INCORRECT;
    }
    
    return propperWrapper->WriteFile(fileName, imageSet, compressionFormat, isCubeMap);
}

eErrorCode ImageSystem::Save(const FilePath & fileName, Image *image, PixelFormat compressionFormat, bool isCubeMap)
{
    if (NULL == image)
    {
        return ERROR_WRITE_FAIL;
    }
    Vector<Image*> imageSet;
    imageSet.push_back(image);
    return Save(fileName, imageSet, compressionFormat, isCubeMap);
}
    
ImageFormatInterface* ImageSystem::DetectImageFormatInterfaceByExtension(const FilePath & pathname)
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

ImageFormatInterface* ImageSystem::DetectImageFormatInterfaceByContent(File *file)
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
    
};
