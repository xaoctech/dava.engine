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


#include "ui_ImageSplitter.h"
#include "ImageSplitterDialog/ImageSplitterDialog.h"
#include "Project/ProjectManager.h"
#include <QMessageBox>

ImageSplitterDialog::ImageSplitterDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ImageSplitter),
    acceptableSize(0,0)
{
    ui->setupUi(this);
    ui->selectPathWidget->SetClearButtonVisible(false);
    ui->selectPathWidget->setAcceptDrops(false);
    ui->selectPathWidget->SetFileFormatFilter("PNG(*.png)");
    DAVA::FilePath defaultPath = ProjectManager::Instance()->CurProjectPath().toStdString();
    ui->selectPathWidget->SetOpenDialogDefaultPath(defaultPath);
    ui->redImgLbl->SetColorComponent(ImageArea::COMPONENTS_RED);
    ui->greenImgLbl->SetColorComponent(ImageArea::COMPONENTS_GREEN);
    ui->blueImgLbl->SetColorComponent(ImageArea::COMPONENTS_BLUE);
    ui->alphaImgLbl->SetColorComponent(ImageArea::COMPONENTS_ALPHA);
    ui->saveBtn->setFocus();
    ConnectSignals();
}

ImageSplitterDialog::~ImageSplitterDialog()
{
    delete ui;
}

void ImageSplitterDialog::ConnectSignals()
{
    connect(ui->selectPathWidget, SIGNAL(PathSelected(DAVA::String)), this, SLOT(PathSelected(DAVA::String)));

    connect(ui->redImgLbl, SIGNAL(changed()), this, SLOT(ImageAreaChanged()));
    connect(ui->greenImgLbl, SIGNAL(changed()), this, SLOT(ImageAreaChanged()));
    connect(ui->blueImgLbl, SIGNAL(changed()), this, SLOT(ImageAreaChanged()));
    connect(ui->alphaImgLbl, SIGNAL(changed()), this, SLOT(ImageAreaChanged()));
}

void ImageSplitterDialog::PathSelected(DAVA::String path)
{
    DAVA::FilePath imagePath(path);
    if(!imagePath.Exists())
    {
        return;
    }
    DAVA::Vector<DAVA::Image*> images = DAVA::ImageLoader::CreateFromFileByContent(imagePath);
    if (images.size() == 0)
    {
        QMessageBox::warning(this, "File error", "Cann't load image.", QMessageBox::Ok);
        return;
    }
    DAVA::Vector2 size(images[0]->GetWidth(), images[0]->GetHeight());
    for_each(images.begin(), images.end(), DAVA::SafeRelease<DAVA::Image>);
    SetAcceptableImageSize(size);
    ui->redImgLbl->SetImage(imagePath);
    ui->greenImgLbl->SetImage(imagePath);
    ui->blueImgLbl->SetImage(imagePath);
    ui->alphaImgLbl->SetImage(imagePath);
}

void ImageSplitterDialog::ImageAreaChanged()
{
    ImageArea* sender = dynamic_cast<ImageArea*>(QObject::sender());
    
    DAVA::Vector2 senderImageSize  = sender->GetAcceptableSize();
    if(acceptableSize.IsZero())
    {
        SetAcceptableImageSize(senderImageSize);
    }
}

void ImageSplitterDialog::SetAcceptableImageSize(const DAVA::Vector2& newSize)
{
    acceptableSize = newSize;
    DAVA::String lblText = DAVA::Format("%.f * %.f", acceptableSize.x, acceptableSize.y);
    ui->ImageSizeLbl->setText(lblText.c_str());
    ui->redImgLbl->SetAcceptableSize(acceptableSize);
    ui->greenImgLbl->SetAcceptableSize(acceptableSize);
    ui->blueImgLbl->SetAcceptableSize(acceptableSize);
    ui->alphaImgLbl->SetAcceptableSize(acceptableSize);
}
