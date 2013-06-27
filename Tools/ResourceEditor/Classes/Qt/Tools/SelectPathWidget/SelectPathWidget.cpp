/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA, INC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA, INC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "SelectPathWidget.h"
#include "ui_SelectPathWidget.h"
#include "EditorSettings.h"
#include "./../Qt/Main/QtUtils.h"
#include "./../Qt/DockSceneTree/SceneTreeModel.h"


#include <QFileInfo>
#include <QKeyEvent>
#include <QUrl>
#include <QFileDialog>

SelectPathWidget::SelectPathWidget(QWidget* parent)
:	QWidget(parent),
ui(new Ui::SelectPathWidget)
{
	ui->setupUi(this);
    setAcceptDrops(true);
    
    connect(ui->EraseBtn, SIGNAL(clicked()), this, SLOT(EraseClicked()));
    
    connect(ui->SelectDialogBtn, SIGNAL(clicked()), this, SLOT(OpenClicked()));
    
    //
	//connect(ui->xAxisModify, SIGNAL(editingFinished()), this, SLOT(OnEditingFinished()));
	//connect(ui->yAxisModify, SIGNAL(editingFinished()), this, SLOT(OnEditingFinished()));
	//connect(ui->zAxisModify, SIGNAL(editingFinished()), this, SLOT(OnEditingFinished()));
    
    //FilePath dataSourcePath = EditorSettings::Instance()->GetDataSourcePath();
    //selectedScenePathname = GetOpenFileName(String("Open Scene File"), dataSourcePath, String("Scene File (*.sc2)"));
    /*
     const char* SceneTreeModel::mimeFormatEntity = "application/dava.entity";
     const char* SceneTreeModel::mimeFormatEmitter = "application/dava.emitter";
     */
    
}

SelectPathWidget::~SelectPathWidget()
{
	delete ui;
}

void SelectPathWidget::dragEnterEvent(QDragEnterEvent* event)
{
    if(true)//TODO: MimeDataHelper::IsAcceptedMimeData(event->mimeData())
    {
        event->acceptProposedAction();;
    }
    /*
    
    QStringList t = event->mimeData()->formats();
    for (int i = 0; i < t.size(); ++i)
    {
        String dStr (t.at(i).toStdString());
        if(strcmp(t.at(i).toStdString().c_str(), SceneTreeModel::mimeFormatEntity) == 0 )
        {
            
            
        }
        int k = 0;
    }
    
 
 
    const QMimeData* mimeData = event->mimeData();

    
	if(mimeData->hasFormat(SceneTreeModel::mimeFormatEntity))
	{

		DAVA::Entity *entity = NULL;

		QByteArray encodedData = mimeData->data(SceneTreeModel::mimeFormatEntity);
		QDataStream stream(&encodedData, QIODevice::ReadOnly);
		EntityGroup entityGroup;
        
		while(!stream.atEnd())
		{
			stream.readRawData((char *) &entity, sizeof(DAVA::Entity*));
			if(NULL != entity)
			{
				//entity->
			}
		}
	}
	//ret = QStandardItemModel::dropMimeData(data, action, row, column, parent);
    
    //if(event->provides(SceneTreeModel::mimeFormatEntity))
      
    
    
    
    
    if(event->mimeData()->hasUrls())
    {
        event->acceptProposedAction();
    }*/
   
}

void SelectPathWidget::dropEvent(QDropEvent* event)
{
    
    
    const QMimeData* sendedMimeData = event->mimeData();
    if(sendedMimeData->hasUrls())
    {
        QList<QUrl> droppedUrls = event->mimeData()->urls();
        int droppedUrlCnt = droppedUrls.size();
        //TODO: remove loop
        for(int i = 0; i < droppedUrlCnt; i++)
        {
            QString localPath = droppedUrls[i].toLocalFile();
            QFileInfo fileInfo(localPath);
            
            if(fileInfo.isFile() && fileInfo.completeSuffix() == "sc2")//!
            {
                SetPathText(localPath);
            }
        }
        
    }
    
    
    //mimeData = *event->mimeData();
    
    mimeData.clear();
    
    
    //copy of mimeData maybe to mimeDataHelper!
    //TODO: SetPathText with fist file name
    
    
    foreach(const QString & format, event->mimeData()->formats())
    {
        mimeData.setData(format, event->mimeData()->data(format));
    }
    
    event->acceptProposedAction();
}


void SelectPathWidget::EraseClicked()
{
    ui->FilePathBox->setText("");
}

void SelectPathWidget::OpenClicked()
{
    FilePath presentPath(ui->FilePathBox->text().toStdString());
    FilePath dialogString = EditorSettings::Instance()->GetDataSourcePath();
    if(presentPath.GetDirectory().Exists())
    {
        dialogString = presentPath.GetDirectory();
    }
    
    QString filePath = QFileDialog::getOpenFileName(NULL, "Open Scene File", QString(dialogString.GetAbsolutePathname().c_str()),
                                                    "Scene File (*.sc2)");
 
    SetPathText(filePath);
    //TODO: mimeData = mimeDataHelper::ConvertToMime(List<FilePath> )
    
    mimeData.clear();
    
    QList<QUrl> list;
    list.append(QUrl(filePath));
    mimeData.setUrls(list);
}


void SelectPathWidget::SetDiscriptionText(const QString& discription)
{
    ui->DiscriptionLabel->setText(discription);
}

QString SelectPathWidget::GetDiscriptionText()
{
    return ui->DiscriptionLabel->text();
}

void SelectPathWidget::SetPathText(const QString& filePath)
{
    ui->FilePathBox->setText(ConvertToRelativPath(filePath));
}


QString SelectPathWidget::GetPathText()
{
    return ui->FilePathBox->text();
}

void SelectPathWidget::SetRelativePath(const QString& newRelativPath)
{
    relativPath = newRelativPath;
    QString existingPath = ui->FilePathBox->text();
    if(!existingPath.isEmpty())
    {
        ui->FilePathBox->setText(ConvertToRelativPath(existingPath));
    }
}

QString SelectPathWidget::ConvertToRelativPath(const QString& path)
{
    //QString retValue = path;
    int index = path.indexOf(relativPath);
    QString retValue = path.right(path.length() - index);
    return retValue;
}
