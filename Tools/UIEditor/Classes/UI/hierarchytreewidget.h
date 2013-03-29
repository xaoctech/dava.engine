#ifndef HIERARCHYTREEWIDGET_H
#define HIERARCHYTREEWIDGET_H

#include <QWidget>
#include <QTreeWidgetItem>
#include "HierarchyTreeControlNode.h"
#include "HierarchyTreeController.h"


namespace Ui {
class HierarchyTreeWidget;
}

class HierarchyTreeWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit HierarchyTreeWidget(QWidget *parent = 0);
    ~HierarchyTreeWidget();
	
private:
	typedef Set<HierarchyTreeNode::HIERARCHYTREENODEID> EXPANDEDITEMS;
	void AddControlItem(QTreeWidgetItem* parent, const EXPANDEDITEMS& selectedItems,const EXPANDEDITEMS& expandedItems,  const HierarchyTreeNode::HIERARCHYTREENODESLIST& items);
    
signals:
	void CreateNewScreen(HierarchyTreeNode::HIERARCHYTREENODEID platformId);
	
protected slots:
	void OnTreeUpdated();
	
private slots:
    void on_treeWidget_itemSelectionChanged();
	void OnSelectedControlNodesChanged(const HierarchyTreeController::SELECTEDCONTROLNODES &);
	void OnShowCustomMenu(const QPoint& pos);
	void OnDeleteControlAction();
	void OnCreateScreenAction();
	void OnCopyAction();
	void OnPasteAction();
	
private:
	typedef Map<int, QTreeWidgetItem*> TREEITEMS;
	TREEITEMS GetAllItems();
	void GetChildItems(const QTreeWidgetItem* parent, Map<int, QTreeWidgetItem*> &items);
	void ResetSelection();

	void DecorateWithIcon(QTreeWidgetItem* item, UIControl* uiControl);

private:
	bool internalSelectionChanged;
    Ui::HierarchyTreeWidget *ui;
};

#endif // HIERARCHYTREEWIDGET_H
