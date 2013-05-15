#include <QMimeData>

#include "DockSceneTree/SceneTreeModel.h"

SceneTreeModel::SceneTreeModel(QObject* parent /*= 0*/ )
	: QStandardItemModel(parent)
	, curScene(NULL)
{
	setColumnCount(1);
	setSupportedDragActions(Qt::MoveAction);

	QStringList headerLabels;
	headerLabels.append("Scene hierarchy");
	setHorizontalHeaderLabels(headerLabels);
}

SceneTreeModel::~SceneTreeModel()
{
	if(NULL != curScene)
	{
		curScene->Release();
		curScene = NULL;
	}
}

void SceneTreeModel::SetScene(SceneEditorProxy *scene)
{
	// remove add rows
	removeRows(0, rowCount());

	if(NULL != curScene)
	{
		curScene->Release();
	}

	curScene = scene;

	if(NULL != curScene)
	{
		curScene->Retain();

		// add new rows
		for(int i = 0; i < scene->GetChildrenCount(); ++i)
		{
			SceneTreeItem *item = new SceneTreeItem(scene->GetChild(i));

			QList<QStandardItem *> row;
			row.push_back(item);

			appendRow(row);
		}
	}
}

SceneEditorProxy* SceneTreeModel::GetScene() const
{
	return curScene;
}

QModelIndex SceneTreeModel::GetEntityIndex(DAVA::Entity *entity) const
{
	QModelIndex index;
	SceneTreeItem *foundItem = NULL;

	for(int i = 0; i < rowCount(); ++i)
	{
		foundItem = ((SceneTreeItem *) item(i))->SearchEntity(entity);
		if(NULL != foundItem)
		{
			break;
		}
	}

	if(NULL != foundItem)
	{
		index = indexFromItem(foundItem);
	}

	return index;
}

DAVA::Entity* SceneTreeModel::GetEntity(const QModelIndex &index) const
{
	DAVA::Entity *ret = NULL;

	SceneTreeItem *item = (SceneTreeItem *) itemFromIndex(index);
	if(NULL != item)
	{
		ret = item->GetEntity();
	}

	return ret;
}

Qt::DropActions SceneTreeModel::supportedDropActions() const
{
	return Qt::MoveAction;
}

QMimeData * SceneTreeModel::mimeData(const QModelIndexList & indexes) const
{
	QMimeData *mimeData = new QMimeData();
	QByteArray encodeData;

	QDataStream stream(&encodeData, QIODevice::WriteOnly);
	foreach(QModelIndex index, indexes)
	{
		if(index.isValid())
		{
			DAVA::Entity* entity = GetEntity(index);
			stream.writeBytes((char *) &entity, sizeof(DAVA::Entity*));
		}
	}

	mimeData->setData("application/dava.entity", encodeData);
	return mimeData;
}

QStringList SceneTreeModel::mimeTypes() const
{
	QStringList types;

	types << "application/dava.entity";

	return types;
}

bool SceneTreeModel::dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent)
{
	return false;
	//return QStandardItemModel::dropMimeData(data, action, row, column, parent);
}
