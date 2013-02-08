//
//  PropertyEditorStateHelper.h
//  ResourceEditorQt
//
//  Created by Yuri Coder on 2/7/13.
//
//

#ifndef __PROPERTY_EDITOR_STATE_HELPER__H__
#define __PROPERTY_EDITOR_STATE_HELPER__H__

#include "../Main/QTreeViewStateHelper.h"
#include "../QtPropertyEditor/QtPropertyModel.h"

class PropertyEditorStateHelper : public DAVA::QTreeViewStateHelper<QString>
{
public:
	PropertyEditorStateHelper(QTreeView* treeView, QtPropertyModel* model);
	
protected:
	virtual QString GetPersistentDataForModelIndex(const QModelIndex &modelIndex);
	
private:
	QtPropertyModel* model;
};

#endif /* defined(__PROPERTY_EDITOR_STATE_HELPER__H__) */
