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



#include "SelectEntityPathWidget.h"
#include "./../Qt/Tools/MimeDataHelper/MimeDataHelper.h"


#include <QFileInfo>
#include <QKeyEvent>
#include <QUrl>
#include <QFileDialog>
#include <QStyle>

SelectEntityPathWidget::SelectEntityPathWidget(QWidget* _parent, DAVA::String _openDialogDefualtPath, DAVA::String _relativPath)
:	SelectPathWidgetBase(_parent, _openDialogDefualtPath, _relativPath, "Open Scene File", "Scene File (*.sc2)")
{
}

void SelectEntityPathWidget::dragEnterEvent(QDragEnterEvent* event)
{
	if(DAVA::MimeDataHelper::IsMimeDataTypeSupported(event->mimeData()))
	{
		event->acceptProposedAction();
	}
}

void SelectEntityPathWidget::dropEvent(QDropEvent* event)
{
	const QMimeData* sendedMimeData = event->mimeData();
		
	DAVA::List<DAVA::String> nameList;
	
	DAVA::MimeDataHelper::GetItemNamesFromMimeData(sendedMimeData, nameList);
	if(nameList.size() == 0)
	{
		return;
	}
	
	mimeData.clear();

	
	foreach(const QString & format, event->mimeData()->formats())
	{
		if(DAVA::MimeDataHelper::IsMimeDataTypeSupported(format.toStdString()))
		{
			mimeData.setData(format, event->mimeData()->data(format));
		}
	}
	
	DAVA::String itemName = *nameList.begin();
	DAVA::FilePath filePath(itemName);
	if(filePath.Exists())// check is it item form scene tree or file system
	{
		setText(filePath.GetAbsolutePathname());
	}
	else
	{
		setText(itemName);
	}
	
	event->acceptProposedAction();
}

void SelectEntityPathWidget::EraseWidget()
{
	setText(QString(""));
	mimeData.clear();
}

void SelectEntityPathWidget::HandlePathSelected(DAVA::String name)
{
	SelectPathWidgetBase::HandlePathSelected(name);
	
	DAVA::FilePath fullPath(name);
	DAVA::List<DAVA::FilePath> urls;
	urls.push_back(fullPath);
	DAVA::MimeDataHelper::ConvertToMimeData(urls, &mimeData);
}

DAVA::Entity* SelectEntityPathWidget::GetOutputEntity(SceneEditor2* editor)
{
	DAVA::List<DAVA::Entity*> retList;
	DAVA::MimeDataHelper::ConvertFromMimeData(&mimeData, retList, editor);
	DAVA::Entity* retEntity = retList.size() > 0 ? *retList.begin(): NULL;
	return retEntity;
}
