//
//  QTreeViewStateHelper.cpp
//  ResourceEditorQt
//
//  Created by Yuri Coder on 2/4/13.
//
//

#include "QTreeViewStateHelper.h"
using namespace DAVA;

QTreeViewStateHelper::QTreeViewStateHelper(QTreeView* treeView)
{
	this->treeView = treeView;
}

void QTreeViewStateHelper::SaveTreeViewState()
{
	if (!treeView || !treeView->model())
	{
		return;
	}

	SaveTreeViewStateRecursive(treeView, QModelIndex(), internalStorage);
}

void QTreeViewStateHelper::RestoreTreeViewState()
{
	if (!treeView || !treeView->model())
	{
		return;
	}
	
	RestoreTreeViewStateRecursive(treeView, QModelIndex(), internalStorage);
}

void QTreeViewStateHelper::SaveTreeViewState(Map<void*, bool>& storage)
{
	if (!treeView || !treeView->model())
	{
		return;
	}
	
	storage.clear();
	SaveTreeViewStateRecursive(treeView, QModelIndex(), storage);
}

void QTreeViewStateHelper::RestoreTreeViewState(Map<void*, bool>& storage)
{
	if (!treeView || !treeView->model())
	{
		return;
	}
	
	if (storage.empty())
	{
		return;
	}

	RestoreTreeViewStateRecursive(treeView, QModelIndex(), storage);
}

void QTreeViewStateHelper::SaveTreeViewStateRecursive(const QTreeView* treeView, const QModelIndex &parent, Map<void*, bool>& storage)
{
	QAbstractItemModel* model = treeView->model();
    int rowCount = model->rowCount(parent);

    for(int i = 0; i < rowCount; ++i)
    {
        QPersistentModelIndex idx = model->index(i, 0, parent);
	    if (!idx.isValid())
        {
			continue;
		}
		
		void* persistentData = GetPersistentDataForModelIndex(idx);
		if (persistentData)
		{
			bool isExpanded = treeView->isExpanded(idx);
			storage.insert(std::make_pair(persistentData, isExpanded));
		}

        SaveTreeViewStateRecursive(treeView, idx, storage);
    }
}

void QTreeViewStateHelper::RestoreTreeViewStateRecursive(QTreeView* treeView, const QModelIndex &parent, Map<void*, bool>& storage)
{
	QAbstractItemModel* model = treeView->model();
    int rowCount = model->rowCount(parent);
	
    for(int i = 0; i < rowCount; ++i)
    {
        QModelIndex idx = model->index(i, 0, parent);
		if (!idx.isValid())
		{
			continue;
		}

		void* persistentData = GetPersistentDataForModelIndex(idx);
		if (!persistentData)
		{
			continue;
		}

		Map<void*, bool>::iterator iter = storage.find(persistentData);
		if (iter != storage.end())
		{
			bool isExpanded = iter->second;
			treeView->setExpanded(idx, isExpanded);
		}

		RestoreTreeViewStateRecursive(treeView, idx, storage);
    }
}
