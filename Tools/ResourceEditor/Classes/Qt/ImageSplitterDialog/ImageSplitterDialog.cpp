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

ImageSplitterDialog::ImageSplitterDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ImageSplitter)
{
    ui->setupUi(this);
    ui->selectPathWidget->SetClearButtonVisible(false);
    DAVA::List<DAVA::String> allowedFormats;
	allowedFormats.push_back(".png");
    ui->selectPathWidget->SetAllowedFormatsList(allowedFormats);
    ui->selectPathWidget->SetFileFormatFilter("PNG(*.png)");
    DAVA::FilePath defaultPath = ProjectManager::Instance()->CurProjectPath().toStdString();
    ui->selectPathWidget->SetOpenDialogDefaultPath(defaultPath);
    ui->redImgLbl->SetColorComponent(ImageArea::COMPONENTS_RED);
    ui->greenImgLbl->SetColorComponent(ImageArea::COMPONENTS_GREEN);
    ui->blueImgLbl->SetColorComponent(ImageArea::COMPONENTS_BLUE);
    ui->alphaImgLbl->SetColorComponent(ImageArea::COMPONENTS_ALPHA);
    ConnectSignals();
}

ImageSplitterDialog::~ImageSplitterDialog()
{
    delete ui;
}

void ImageSplitterDialog::ConnectSignals()
{
    connect(ui->selectPathWidget, SIGNAL(PathSelected(DAVA::String)),
			this, SLOT(PathSelected(DAVA::String)));
}

void ImageSplitterDialog::PathSelected(DAVA::String path)
{
    DAVA::FilePath fp(path);
    
    ui->redImgLbl->SetImage(fp);
    ui->greenImgLbl->SetImage(fp);
    ui->blueImgLbl->SetImage(fp);
    ui->alphaImgLbl->SetImage(fp);
}
