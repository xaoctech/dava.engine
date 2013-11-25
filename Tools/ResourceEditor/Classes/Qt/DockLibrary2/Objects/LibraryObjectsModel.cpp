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



#include "LibraryObjectsModel.h"

#include "Main/mainwindow.h"
#include "Main/QtUtils.h"
#include "Scene/SceneTabWidget.h"
#include "Scene/SceneEditor2.h"
#include "Commands2/DAEConvertAction.h"

LibraryObjectsModel::LibraryObjectsModel()
    : LibraryFileSystemModel("Objects")
{
	CreateActions();
	SetNameFilters();
}

void LibraryObjectsModel::TreeFileSelected(const QFileInfo & fileInfo)
{
    
}

void LibraryObjectsModel::ListFileSelected(const QFileInfo & fileInfo)
{
    if(0 == fileInfo.suffix().compare("sc2", Qt::CaseInsensitive))
    {
        ShowPreview(fileInfo.filePath().toStdString());
    }
    else
    {
        HidePreview();
    }
}

bool LibraryObjectsModel::PrepareTreeContextMenuInternal(QMenu &contextMenu, const QFileInfo & fileInfo) const
{
    return false;
}

bool LibraryObjectsModel::PrepareListContextMenuInternal(QMenu &contextMenu, const QFileInfo & fileInfo) const
{
    HidePreview();
    
    QString fileExtension = fileInfo.suffix();
    
    QVariant fileInfoAsVariant = QVariant::fromValue<QFileInfo>(fileInfo);
    if(0 == fileExtension.compare("sc2", Qt::CaseInsensitive))
    {
        QAction * actionAdd = contextMenu.addAction("Add Model", this, SLOT(OnModelAdd()));
        QAction * actionEdit = contextMenu.addAction("Edit Model", this, SLOT(OnModelEdit()));
        
        actionAdd->setData(fileInfoAsVariant);
        actionEdit->setData(fileInfoAsVariant);
        
        return true;
    }
    else if(0 == fileExtension.compare("dae", Qt::CaseInsensitive))
    {
        QAction * actionConvert = contextMenu.addAction("Convert", this, SLOT(OnDAEConvert()));
        QAction * actionConvertGeometry = contextMenu.addAction("Convert geometry", this, SLOT(OnDAEConvertGeometry()));
        
        actionConvert->setData(fileInfoAsVariant);
        actionConvertGeometry->setData(fileInfoAsVariant);
        
        return true;
    }

    return false;
}


void LibraryObjectsModel::OnModelEdit()
{
    QVariant indexAsVariant = ((QAction *)sender())->data();
    const QFileInfo fileInfo = indexAsVariant.value<QFileInfo>();
 
    QtMainWindow::Instance()->OpenScene(fileInfo.absoluteFilePath());
}

void LibraryObjectsModel::OnModelAdd()
{
    QVariant indexAsVariant = ((QAction *)sender())->data();
    const QFileInfo fileInfo = indexAsVariant.value<QFileInfo>();
    
    SceneEditor2 *scene = QtMainWindow::Instance()->GetCurrentScene();
    if(NULL != scene)
    {
        QtMainWindow::Instance()->WaitStart("Add object to scene", fileInfo.absoluteFilePath());

        scene->structureSystem->Add(fileInfo.absoluteFilePath().toStdString());
        
        QtMainWindow::Instance()->WaitStop();
    }
}

void LibraryObjectsModel::OnDAEConvert()
{
    QVariant indexAsVariant = ((QAction *)sender())->data();
    const QFileInfo fileInfo = indexAsVariant.value<QFileInfo>();
    
    QtMainWindow::Instance()->WaitStart("DAE to SC2 Conversion", fileInfo.absoluteFilePath());
    
    Command2 *daeCmd = new DAEConvertAction(fileInfo.absoluteFilePath().toStdString());
    daeCmd->Redo();
    delete daeCmd;
    
    QtMainWindow::Instance()->WaitStop();
}

void LibraryObjectsModel::OnDAEConvertGeometry()
{
    QVariant indexAsVariant = ((QAction *)sender())->data();
    const QFileInfo fileInfo = indexAsVariant.value<QFileInfo>();

    QtMainWindow::Instance()->WaitStart("DAE to SC2 Conversion", fileInfo.absoluteFilePath());
    
    Command2 *daeCmd = new DAEConvertWithSettingsAction(fileInfo.absoluteFilePath().toStdString());
    daeCmd->Redo();
    delete daeCmd;
    
    QtMainWindow::Instance()->WaitStop();
}


void LibraryObjectsModel::HidePreview() const
{
    SceneTabWidget *widget = QtMainWindow::Instance()->GetSceneWidget();
    widget->HideScenePreview();
}

void LibraryObjectsModel::ShowPreview(const DAVA::FilePath & pathname) const
{
    SceneTabWidget *widget = QtMainWindow::Instance()->GetSceneWidget();
    widget->ShowScenePreview(pathname);
}

void LibraryObjectsModel::CreateActions()
{
	QIcon daeIcon(QString::fromUtf8(":/QtLibraryIcons/daeIcon"));
	QIcon sc2Icon(QString::fromUtf8(":/QtLibraryIcons/sc2Icon"));

	showDAE = CreateAction(daeIcon, "Display *.dae", "*.dae");
	showSC2 = CreateAction(sc2Icon, "Display *.sc2", "*.sc2");
}

