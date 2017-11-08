#include <Render/Image/ImageConverter.h>

namespace DAVA
{
void ImageConverter::SetImplementation(ImageConverter* converter)
{
    implementation = converter;
}

bool ImageConverter::CanConvert(PixelFormat srcFormat, PixelFormat dstFormat) const
{
    if (implementation != nullptr)
    {
        return implementation->CanConvertImpl(srcFormat, dstFormat);
    }
    return false;
}

bool ImageConverter::Convert(const Image* srcImage, Image* dstImage) const
{
    if (implementation != nullptr)
    {
        return implementation->ConvertImpl(srcImage, dstImage);
    }
    return false;
}

bool ImageConverter::CanConvertImpl(PixelFormat srcFormat, PixelFormat dstFormat) const
{
    return false;
}

bool ImageConverter::ConvertImpl(const Image* srcImage, Image* dstImage) const
{
    return false;
}

} //DAVA
