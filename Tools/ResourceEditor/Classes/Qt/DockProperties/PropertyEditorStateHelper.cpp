//
//  PropertyEditorStateHelper.cpp
//  ResourceEditorQt
//
//  Created by Yuri Coder on 2/7/13.
//
//

#include "PropertyEditorStateHelper.h"

PropertyEditorStateHelper::PropertyEditorStateHelper(QTreeView* treeView, QtPropertyModel* model) :
	QTreeViewStateHelper(treeView)
{
	this->model = model;
}

void PropertyEditorStateHelper::SaveTreeViewState(bool needCleanupStorage)
{
	// Need to cleanup the full paths cache before the save.
	this->fullPathsCache.clear();
	DAVA::QTreeViewStateHelper<QString>::SaveTreeViewState(needCleanupStorage);
}

QString PropertyEditorStateHelper::GetPersistentDataForModelIndex(const QModelIndex &modelIndex)
{
	if (!this->model)
	{
		return QString();
	}

	// Calculate the full path up to the root. An optimization is required here - since this
	// method is called recursively, we must already know the full path to the parent, just
	// append the child name to it.
	QString fullPath;
	QStandardItem* item = model->itemFromIndex(modelIndex);
	if (!item)
	{
		return fullPath;
	}

	if (item->parent())
	{
		DAVA::Map<QStandardItem*, QString>::iterator parentIter = fullPathsCache.find(item->parent());
		if (parentIter != fullPathsCache.end())
		{
			fullPath = parentIter->second;
		}
	}

	// Append the current node name and store the full path to it in the cache.
	fullPath = fullPath + "//" + item->data(Qt::DisplayRole).toString();
	fullPathsCache[item] = fullPath;

	return fullPath;
}
