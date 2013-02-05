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
	
class QTreeViewStateHelper
{
public:
	QTreeViewStateHelper(QTreeView* treeView);

	void SaveTreeViewState();
	void RestoreTreeViewState();
	
	// Extended versions - save/restore Tree View state to external storage.
	void SaveTreeViewState(Map<void*, bool>& storage);
	void RestoreTreeViewState(Map<void*, bool>& storage);

protected:
	// This method must be overriden to return persistent data to particular model index.
	virtual void* GetPersistentDataForModelIndex(const QModelIndex &modelIndex) = 0;

	void SaveTreeViewStateRecursive(const QTreeView* treeView, const QModelIndex &parent,
									Map<void*, bool>& storage);
	void RestoreTreeViewStateRecursive(QTreeView* treeView, const QModelIndex &parent,
									   Map<void*, bool>& storage);
	
private:
	// QTreeView to attach to.
	QTreeView* treeView;

	// Collapsed items.
	Map<void*, bool> internalStorage;
};

};

#endif //__QTREEVIEW_STATE_HELPER__H__
