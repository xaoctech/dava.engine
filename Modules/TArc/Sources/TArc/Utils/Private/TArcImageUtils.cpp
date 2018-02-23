#include "TArc/Utils/TArcImageUtils.h"

#include <Base/ScopedPtr.h>
#include <Base/GlobalEnum.h>
#include <Logger/Logger.h>
#include <Render/Image/Image.h>
#include <Render/Image/ImageConvert.h>
#include <Render/RenderBase.h>

namespace DAVA
{
QImage TArcImageUtils::FromDavaImage(const Image* image)
{
    DVASSERT(image != nullptr);

    if (image->format == PixelFormat::FORMAT_RGBA8888)
    {
        QImage qtImage(image->width, image->height, QImage::Format_RGBA8888);
        Memcpy(qtImage.bits(), image->data, image->dataSize);
        return qtImage;
    }
    else if (ImageConvert::CanConvertFromTo(image->format, PixelFormat::FORMAT_RGBA8888))
    {
        ScopedPtr<Image> newImage(Image::Create(image->width, image->height, PixelFormat::FORMAT_RGBA8888));
        bool converted = ImageConvert::ConvertImage(image, newImage);
        if (converted)
        {
            return FromDavaImage(newImage);
        }
        else
        {
            Logger::Error("[%s]: Error converting from %s", __FUNCTION__, GlobalEnumMap<PixelFormat>::Instance()->ToString(image->format));
            return QImage();
        }
    }
    else
    {
        Logger::Error("[%s]: Converting from %s is not implemented", __FUNCTION__, GlobalEnumMap<PixelFormat>::Instance()->ToString(image->format));
        return QImage();
    }
}

} // namespace DAVA
