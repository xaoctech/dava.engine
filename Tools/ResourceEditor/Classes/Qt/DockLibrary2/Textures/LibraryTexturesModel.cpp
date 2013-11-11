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



#include "LibraryTexturesModel.h"

#include "Main/mainwindow.h"
#include "Main/QtUtils.h"
#include "Scene/SceneTabWidget.h"
#include "Scene/SceneEditor2.h"

#include <QFileSystemModel>
#include <QProcess>


LibraryTexturesModel::LibraryTexturesModel()
    : LibraryFileSystemModel("Textures")
{
	CreateActions();
	SetNameFilters();
}



void LibraryTexturesModel::TreeFileSelected(const QFileInfo & fileInfo)
{
    
}

void LibraryTexturesModel::ListFileSelected(const QFileInfo & fileInfo)
{
}

bool LibraryTexturesModel::PrepareTreeContextMenuInternal(QMenu &contextMenu, const QFileInfo & fileInfo) const
{
    return false;
}

bool LibraryTexturesModel::PrepareListContextMenuInternal(QMenu &contextMenu, const QFileInfo & fileInfo) const
{
    QString fileExtension = fileInfo.suffix();
    
    QVariant fileInfoAsVariant = QVariant::fromValue<QFileInfo>(fileInfo);
    if(0 == fileExtension.compare("tex", Qt::CaseInsensitive))
    {
        QAction * actionEdit = contextMenu.addAction("Edit", this, SLOT(OnEdit()));
        actionEdit->setData(fileInfoAsVariant);
        return true;
    }
    else if(0 == fileExtension.compare("png", Qt::CaseInsensitive))
    {
        QAction * actionEdit = contextMenu.addAction("Edit", this, SLOT(OnEdit()));
        actionEdit->setData(fileInfoAsVariant);
        return true;
    }

    return false;
}

void LibraryTexturesModel::OnEdit()
{
    QVariant indexAsVariant = ((QAction *)sender())->data();
    const QFileInfo fileInfo = indexAsVariant.value<QFileInfo>();
}


void LibraryTexturesModel::CreateActions()
{
    QIcon texIcon(QString::fromUtf8(":/QtLibraryIcons/texIcon"));
	QIcon pngIcon(QString::fromUtf8(":/QtLibraryIcons/pngIcon"));
    QIcon pvrIcon(QString::fromUtf8(":/QtLibraryIcons/pvrIcon"));
	QIcon ddsIcon(QString::fromUtf8(":/QtLibraryIcons/ddsIcon"));
    
	showTEX = CreateAction(texIcon, "Display *.tex", "*.tex");
	showPNG = CreateAction(pngIcon, "Display *.png", "*.png");
	showPVR = CreateAction(pvrIcon, "Display *.pvr", "*.pvr");
	showDDS = CreateAction(ddsIcon, "Display *.dds", "*.dds");
}

