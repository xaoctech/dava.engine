#pragma once

#include <Render/RenderBase.h>
#include <Render/Image/ImageConverter.h>
#include <Render/PixelFormatDescriptor.h>

namespace DAVA
{
class Image;
class ImageConverterImpl : public ImageConverter
{
protected:
    bool CanConvertImpl(PixelFormat srcFormat, PixelFormat dstFormat) const;
    bool ConvertImpl(const Image* srcImage, Image* dstImage) const;
};

} //DAVA
