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


#include "ImageArea.h"

#include "CommandLine/ImageSplitter/ImageSplitter.h"
#include "Qt/TextureBrowser/TextureConvertor.h"

#include <QtGui>

ImageArea::ImageArea(QWidget *parent /*= 0*/, eColorCmponents value /*= COMPONENTS_ALL*/)
    :QLabel(parent),
    colorComponent(value)
{
    setFrameStyle(QFrame::Sunken | QFrame::StyledPanel);
    setAcceptDrops(true);
    setAutoFillBackground(true);
    ConnectSignals();
    image = NULL;
    clear();
}

ImageArea::~ImageArea()
{
    DAVA::SafeRelease(image);
}

void ImageArea::dragEnterEvent(QDragEnterEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    
    if (!mimeData->hasFormat("text/uri-list"))
    {
        return;
    }
    
    DAVA::Vector<DAVA::Image*> images = DAVA::ImageLoader::CreateFromFileByContent(mimeData->urls().first().toLocalFile().toStdString());
    if (images.size() != 0)
    {
        for_each(images.begin(), images.end(), DAVA::SafeRelease<DAVA::Image>);
        event->acceptProposedAction();
    }
}

void ImageArea::dropEvent(QDropEvent *event)
{
    SetImage(event->mimeData()->urls().first().toLocalFile().toStdString());
    event->acceptProposedAction();
}

void ImageArea::ConnectSignals()
{
    connect(this, SIGNAL(changed()), this, SLOT(UpdatePreviewPicture()));
}

void ImageArea::SetColorComponent(eColorCmponents value)
{
    colorComponent = value;
    emit changed();
}

void ImageArea::SetImage(const DAVA::FilePath& filePath)
{
    if(!filePath.Exists())
    {
        return;
    }
    
    DAVA::Vector<DAVA::Image*> images = DAVA::ImageLoader::CreateFromFileByContent(filePath);
    if (images.size() == 0)
    {
        return;
    }
    DAVA::SafeRelease(image);
    image = images[0];
    image->Retain();
    for_each(images.begin(), images.end(), DAVA::SafeRelease<DAVA::Image>);
    emit changed();
}

void ImageArea::clear()
{
    DAVA::SafeRelease(image);
    setBackgroundRole(QPalette::Dark);
    emit changed();
}

DAVA::Image* ImageArea::GetComponentImage( DAVA::Image* originalImage)
{
    if(colorComponent == COMPONENTS_ALL)
    {
        return DAVA::Image::CopyImageRegion(originalImage, originalImage->GetWidth(), originalImage->GetHeight());
    }
    
    DAVA::Image* r = NULL;
    DAVA::Image* g = NULL;
    DAVA::Image* b = NULL;
    DAVA::Image* a = NULL;
    DAVA::Image* outputImage = NULL;
    ImageSplitter::CreateSplittedImages(originalImage, &r, &g, &b, &a);
    switch (colorComponent)
    {
        case COMPONENTS_RED:
            outputImage = r;
            break;
        case COMPONENTS_GREEN:
            outputImage = g;
            break;
        case COMPONENTS_BLUE:
            outputImage = b;
            break;
        case COMPONENTS_ALPHA:
            outputImage = a;
            break;
        default:
            break;
    }
    outputImage->Retain();
    SafeRelease(r);
    SafeRelease(g);
    SafeRelease(b);
    SafeRelease(a);
    return outputImage;
}

void ImageArea::UpdatePreviewPicture()
{
    if(NULL == image)
    {
        return;
    }
    DAVA::Image* scaledImage = DAVA::Image::CopyImageRegion(image, image->GetWidth(), image->GetHeight());
    scaledImage->ResizeImage(this->width(), this->height());
    DAVA::Image* splittedImage = GetComponentImage(scaledImage);
    DAVA::SafeRelease(scaledImage);
    QPixmap scaledPixmap = QPixmap::fromImage(TextureConvertor::FromDavaImage(splittedImage));
    DAVA::SafeRelease(splittedImage);
    setPixmap(scaledPixmap);
}