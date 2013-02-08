//
//  QTreeViewStateHelper.h
//  ResourceEditorQt
//
//  Created by Yuri Coder on 2/4/13.
//
//

#ifndef __QTREEVIEW_STATE_HELPER__H__
#define __QTREEVIEW_STATE_HELPER__H__

#include "DAVAEngine.h"
#include <QTreeView>

// The purpose of QTreeViewStateHelper is to store the current Collapsed/Expanded
// state for the tree view and expand it back.
namespace DAVA {
	
template<class T>
	class QTreeViewStateHelper
{
public:
	QTreeViewStateHelper(QTreeView* treeView)
	{
		this->treeView = treeView;
	}

	virtual void SaveTreeViewState(bool needCleanupStorage = true)
	{
		if (!treeView || !treeView->model())
		{
			return;
		}
		
		SaveTreeViewState(internalStorage, needCleanupStorage);
	}

	virtual void RestoreTreeViewState()
	{
		if (!treeView || !treeView->model())
		{
			return;
		}
		
		RestoreTreeViewStateRecursive(treeView, QModelIndex(), internalStorage);
	}

	// Extended versions - save/restore Tree View state to external storage.
	void SaveTreeViewState(Map<T, bool>& storage, bool needCleanupStorage)
	{
		if (!treeView || !treeView->model())
		{
			return;
		}

		if (needCleanupStorage)
		{
			storage.clear();
		}

		SaveTreeViewStateRecursive(treeView, QModelIndex(), storage);
	}

	void RestoreTreeViewState(Map<T, bool>& storage)
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

	bool IsTreeStateStorageEmpty()
	{
		return this->internalStorage.empty();
	}
	
	void CleanupTreeStateStorage()
	{
		this->internalStorage.cleanup();
	}

protected:
	// This method must be overriden to return persistent data to particular model index.
	virtual T GetPersistentDataForModelIndex(const QModelIndex &modelIndex) = 0;

	void SaveTreeViewStateRecursive(const QTreeView* treeView, const QModelIndex &parent,
									Map<T, bool>& storage)
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
			
			T persistentData = GetPersistentDataForModelIndex(idx);
			if (persistentData != NULL)
			{
				bool isExpanded = treeView->isExpanded(idx);
				storage[persistentData] = isExpanded;
			}
			
			SaveTreeViewStateRecursive(treeView, idx, storage);
		}
	}

	void RestoreTreeViewStateRecursive(QTreeView* treeView, const QModelIndex &parent,
									   Map<T, bool>& storage)
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
			
			T persistentData = GetPersistentDataForModelIndex(idx);
			if (persistentData == NULL)
			{
				continue;
			}
			
			typename Map<T, bool>::iterator iter = storage.find(persistentData);
			if (iter != storage.end())
			{
				bool isExpanded = iter->second;
				treeView->setExpanded(idx, isExpanded);
			}
			
			RestoreTreeViewStateRecursive(treeView, idx, storage);
		}
	}

private:
	// QTreeView to attach to.
	QTreeView* treeView;

	// Collapsed items.
	Map<T, bool> internalStorage;
};

};

#endif //__QTREEVIEW_STATE_HELPER__H__
