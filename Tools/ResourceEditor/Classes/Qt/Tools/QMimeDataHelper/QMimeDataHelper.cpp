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

#include "QMimeDataHelper.h"
#include "../Qt/DockSceneTree/SceneTreeModel.h"
#include "../Qt/Scene/SceneDataManager.h"

#include <QList>
#include <QUrl>
#include <QFileInfo>


#define MIME_HANDLER(data,name, method)\
    if(data->hasFormat(name)){\
        return method(data);}\

const QString QMimeDataHelper::supportedMimeFormats[] =
{
	"application/dava.entity",
	"application/dava.emitter",
	"text/uri-list"
};

DAVA::Entity* QMimeDataHelper::ConvertQMimeDataFromSceneTree(QMimeData* mimeData)
{
	DAVA::Entity *entity = NULL;

	QByteArray encodedData = mimeData->data(SceneTreeModel::mimeFormatEntity);
	QDataStream stream(&encodedData, QIODevice::ReadOnly);

	while(!stream.atEnd())
	{
		stream.readRawData((char *) &entity, sizeof(DAVA::Entity*));
		if(NULL != entity)
		{
			break;
		}
	}
	return entity;
}

DAVA::Entity* QMimeDataHelper::ConvertQMimeDataFromFilePath(QMimeData* mimeData)
{
	if(!mimeData->hasUrls())
    {
		return NULL;
	}
    
	QList<QUrl> droppedUrls = mimeData->urls();
    
    QString localPath = droppedUrls[0].toLocalFile();
    QFileInfo fileInfo(localPath);
    
    if(!(fileInfo.isFile() && fileInfo.completeSuffix() == "sc2"))
    {
        return NULL;
    }
	Entity* entity = NULL;
	entity = SceneDataManager::Instance()->AddScene(FilePath(localPath.toStdString()));
	return entity;
}

DAVA::Entity* QMimeDataHelper::ConvertQMimeData(QMimeData* mimeData)
{
	MIME_HANDLER(mimeData, supportedMimeFormats[MIME_ENTITY],	ConvertQMimeDataFromSceneTree)
	MIME_HANDLER(mimeData, supportedMimeFormats[MIME_FILE_PATH],ConvertQMimeDataFromFilePath)
	//if(mimeData->hasFormat(SceneTreeModel::mimeFormatEntity))
	//{
	//    return ConvertQMimeDataFromSceneTree(mimeData);
	//}
	return NULL;
};
