#pragma once

#include <QImage>

namespace DAVA
{
class Image;
class TArcImageUtils final
{
public:
    static QImage FromDavaImage(const Image* image);
};
} // namespace DAVA
