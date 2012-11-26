#ifndef __MATERIAL_LIST_H__
#define __MATERIAL_LIST_H__

#include <QListView>
#include "MaterialBrowser/MaterialTreeItem.h"
#include "MaterialBrowser/MaterialViewModel.h"

class MaterialView : public QListView
{
	Q_OBJECT

public:
	MaterialView(QWidget *parent = 0);

signals:
	void selected(const QModelIndex &index);

protected:
	void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

private:

};

#endif // __TEXTURE_LIST_H__
