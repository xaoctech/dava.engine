#ifndef __MATERIAL_TREE_H__
#define __MATERIAL_TREE_H__

#include <QTreeView>

class MaterialTree : public QTreeView
{
	Q_OBJECT

public:
	MaterialTree(QWidget *parent = 0);

signals:
	void Selected(const QItemSelection &selected, const QItemSelection &deselected);

protected:
	void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
};

#endif // __MATERIAL_TREE_H__
