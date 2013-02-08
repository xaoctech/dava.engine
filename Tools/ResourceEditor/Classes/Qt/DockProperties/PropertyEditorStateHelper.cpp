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

QString PropertyEditorStateHelper::GetPersistentDataForModelIndex(const QModelIndex &modelIndex)
{
	if (!this->model)
	{
		return QString();
	}
	
	// Calculate the full path up to the root.
	QString fullPath;

	QStandardItem* item = model->itemFromIndex(modelIndex);
	while (item)
	{
		QString nodeName = item->data(Qt::DisplayRole).toString();
		fullPath = nodeName + "\\"+fullPath;
		item = item->parent();
	}

	return fullPath;
}
