//
//  SceneGraphModelStateHelper.cpp
//  ResourceEditorQt
//
//  Created by Yuri Coder on 2/5/13.
//
//

#include "SceneGraphModelStateHelper.h"
using namespace DAVA;

SceneGraphModelStateHelper::SceneGraphModelStateHelper(QTreeView* treeView, SceneGraphModel* model) :
	QTreeViewStateHelper(treeView)
{
	this->model = model;
}

void* SceneGraphModelStateHelper::GetPersistentDataForModelIndex(const QModelIndex &modelIndex)
{
	if (model)
	{
		return model->GetPersistentDataForModelIndex(modelIndex);
	}

	return NULL;
}
