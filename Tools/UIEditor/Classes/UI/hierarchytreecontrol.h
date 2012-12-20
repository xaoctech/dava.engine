#ifndef HIERARCHYTREECONTROL_H
#define HIERARCHYTREECONTROL_H

#include <QTreeWidget>
#include "HierarchyTreeNode.h"

class HierarchyTreeControlMimeData: public QObjectUserData
{
public:
	HierarchyTreeControlMimeData(const QList<QTreeWidgetItem*> items);
	virtual ~HierarchyTreeControlMimeData();
	
	bool IsDropEnable(const HierarchyTreeNode* parentItem) const;
	HierarchyTreeNode::HIERARCHYTREENODESIDLIST GetItems() const;
	
private:
	HierarchyTreeNode::HIERARCHYTREENODESIDLIST items;
};

class HierarchyTreeControl : public QTreeWidget
{
    Q_OBJECT
public:
    explicit HierarchyTreeControl(QWidget *parent = 0);
	
protected:
	virtual void contextMenuEvent(QContextMenuEvent * event);
	
	virtual QMimeData *mimeData(const QList<QTreeWidgetItem*> items) const;
	
	void dropEvent(QDropEvent *event);
	void dragMoveEvent(QDragMoveEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
	    
signals:
	void ShowCustomMenu(const QPoint& pos);
    
public slots:
};

#endif // HIERARCHYTREECONTROL_H
