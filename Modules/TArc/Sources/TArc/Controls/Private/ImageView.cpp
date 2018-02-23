#include "TArc/Controls/ImageView.h"
#include "TArc/Controls/CommonStrings.h"
#include "TArc/Qt/QtString.h"

#include <Base/FastName.h>
#include <Base/ScopedPtr.h>
#include <Base/GlobalEnum.h>
#include <Logger/Logger.h>
#include <Reflection/ReflectedMeta.h>
#include <Render/RenderBase.h>
#include <Render/Image/Image.h>
#include <Render/Image/ImageConvert.h>

#include <QImage>
#include <QVBoxLayout>
#include <QEvent>
#include <QMouseEvent>
#include <QApplication>

namespace DAVA
{
namespace ImageViewDetails
{
QImage FromDavaImage(const Image* image)
{
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

class PreviewImage : public QLabel
{
public:
    PreviewImage(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags())
        : QLabel(parent, f)
    {
        qApp->installEventFilter(this);
    }

    ~PreviewImage() override
    {
        qApp->removeEventFilter(this);
    }

protected:
    bool eventFilter(QObject* object, QEvent* event) override
    {
        if (event->type() == QEvent::MouseButtonPress)
        {
            close();
        }
        return QLabel::eventFilter(object, event);
    }
};
}

ImageView::ImageView(const Params& params, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent)
    : ControlProxyImpl<QLabel>(params, ControlDescriptor(params.fields), wrappersProcessor, model, parent)
{
    SetupControl();
}

ImageView::ImageView(const Params& params, ContextAccessor* accessor, Reflection model, QWidget* parent)
    : ControlProxyImpl<QLabel>(params, ControlDescriptor(params.fields), accessor, model, parent)
{
    SetupControl();
}

void ImageView::SetupControl()
{
    setFrameStyle(QFrame::Sunken | QFrame::StyledPanel);
    setAlignment(Qt::AlignCenter);
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Dark);
}

void ImageView::UpdateControl(const ControlDescriptor& descriptor)
{
    if (descriptor.IsChanged(Fields::Size))
    {
        imageViewSize = GetFieldValue<int32>(Fields::Size, imageViewSize);
    }

    if (descriptor.IsChanged(Fields::Image))
    {
        Image* image = GetFieldValue<Image*>(Fields::Image, static_cast<Image*>(nullptr));
        if (image != nullptr)
        {
            pixmap = QPixmap::fromImage(ImageViewDetails::FromDavaImage(image));
            setPixmap(pixmap.scaled(imageViewSize, imageViewSize, Qt::KeepAspectRatio));
        }
        else
        {
            pixmap = QPixmap();
            setPixmap(pixmap);
        }
    }
}

void ImageView::mouseReleaseEvent(QMouseEvent* event)
{
    QLabel::mouseReleaseEvent(event);

    if (pixmap.isNull() == false && popupView.isNull() == true)
    {
        popupView = new ImageViewDetails::PreviewImage(nullptr, static_cast<Qt::WindowFlags>(Qt::FramelessWindowHint | Qt::SplashScreen));

        popupView->setAttribute(Qt::WA_DeleteOnClose);
        popupView->setFrameStyle(QLabel::Raised | QLabel::Panel);
        popupView->setAlignment(Qt::AlignCenter);

        int previewSize = 256;
        popupView->setFixedSize(previewSize, previewSize);
        popupView->setPixmap(pixmap.scaled(previewSize, previewSize, Qt::KeepAspectRatio));

        QPoint position = event->globalPos() - event->pos() + QPoint(width() / 2, height() / 2) - QPoint(previewSize / 2, previewSize / 2);
        popupView->move(position);

        popupView->show();
        popupView->setFocus();
    }
    else if (popupView.isNull() == false)
    {
        popupView->close();
    }
}
} // namespace DAVA
