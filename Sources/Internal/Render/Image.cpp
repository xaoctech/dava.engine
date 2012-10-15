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
#include "Render/Image.h"
#include "FileSystem/Logger.h"
#include "Platform/SystemTimer.h"
#include "Utils/Utils.h"
#include "Render/LibPngHelpers.h"
#include "FileSystem/File.h"
#include "FileSystem/FileSystem.h"
#include "Render/Texture.h"

#if defined(__DAVAENGINE_IPHONE__) 
#include <CoreGraphics/CoreGraphics.h>
#include <CoreFoundation/CoreFoundation.h>
#elif defined(__DAVAENGINE_MACOS__)
#include <ApplicationServices/ApplicationServices.h>
#endif //PLATFORMS

namespace DAVA 
{
    
//bool Image::isAlphaPremultiplicationEnabled = true;
//bool Image::IsAlphaPremultiplicationEnabled()
//{
//    return isAlphaPremultiplicationEnabled; 
//}
//
//void Image::EnableAlphaPremultiplication(bool isEnabled)
//{
//    isAlphaPremultiplicationEnabled = isEnabled;
//}

Image::Image()
:	data(0)
,	width(0)
,	height(0)
,	format(FORMAT_RGB565)
//,	isAlphaPremultiplied(false)
{
}

Image::~Image()
{
	SafeDeleteArray(data);
}

Image * Image::Create(int32 width, int32 height, PixelFormat format)
{
	Image * image = new Image();
	image->width = width;
	image->height = height;
	image->format = format;
    
    int32 formatSize = Texture::GetPixelFormatSizeInBytes(format);
    if(formatSize)
    {
        image->data = new uint8[width * height * formatSize];
    }
    else 
    {
        Logger::Error("Image::Create trying to create image with wrong format");
    }
    
	return image;
}


    
#if defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)


Image * Image::CreateFromFile(const String & pathName, bool isAlphaPremultiplied)
{
	Image * davaImage = new Image();
	if (1 != LibPngWrapper::ReadPngFile(pathName.c_str(), davaImage))
	{
		SafeRelease(davaImage);
		return 0;
	}
    if (isAlphaPremultiplied)
    {
        if(davaImage->format == FORMAT_RGBA8888) 
        {
            unsigned int * inOutPixel32 = (unsigned int*)davaImage->data;
            for(int i = 0; i < davaImage->width * davaImage->height; ++i)
            {
                unsigned int pixel = *inOutPixel32;

                unsigned int a = (pixel >> 24) & 0xFF;
                unsigned int r = (pixel >> 16) & 0xFF;
                unsigned int g = (pixel >> 8) & 0xFF;
                unsigned int b = pixel & 0xFF;

                {
                    r = r * a / 255;
                    g = g * a / 255;
                    b = b * a / 255;
                }

                *inOutPixel32 = ((a) << 24) | (r << 16) | (g << 8) | b;
                inOutPixel32++;
                //	*inOutPixel32 = ((*inAlphaData) << 24) | pixel;
                //	unsigned int a = *inAlphaData;
                //	inAlphaData++;
            }
//            davaImage->isAlphaPremultiplied = true;
        }
    }
	return davaImage;
};

#else //other platforms

#endif //PLATFORMS	

void Image::Resize(int32 newWidth, int32 newHeight)
{
    if(newWidth>0 && newHeight>0)
    {
        uint8 * newData = NULL;
        uint8 formatSize = Texture::GetPixelFormatSizeInBytes(format);
        
        if(formatSize>0)
        {
            newData = new uint8[newWidth * newHeight * formatSize];
            
            int32 currentLine = 0;
            int32 indexOnLine = 0;
            int32 indexInOldData = 0;
            
            for(int32 i = 0; i < newWidth * newHeight * formatSize; ++i)
            {
                if((currentLine+1)*newWidth*formatSize<=i)
                {
                    currentLine++;
                }
                
                indexOnLine = i - currentLine*newWidth*formatSize;

                if(currentLine<height)
                {
                    // within height of old image
                    if(indexOnLine<width*formatSize)
                    {
                        // we have data in old image for new image
                        indexInOldData = currentLine*width*formatSize + indexOnLine;
                        newData[i] = data[indexInOldData];
                    }
                    else
                    {
                        newData[i] = 0;
                    }
                }
                else
                {
                    newData[i] = 0;
                }
            }
            
            // resized data
            width = newWidth;
            height = newHeight;
            SafeDeleteArray(data);
            data = newData;
        }
    }
}

void Image::Save(const String & filename)
{
	DVASSERT((FORMAT_RGBA8888 == format) || (FORMAT_A8 == format) || (FORMAT_A16 == format));
	LibPngWrapper::WritePngFile(filename.c_str(), width, height, data, format);
}


};
