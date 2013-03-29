#include "Classes/UI/hierarchytreewidget.h"
#include "ui_hierarchytreewidget.h"
#include "HierarchyTreeController.h"
#include "HierarchyTree.h"
#include "HierarchyTreeNode.h"
#include "HierarchyTreePlatformNode.h"
#include "HierarchyTreeAggregatorControlNode.h"
#include "ItemsCommand.h"
#include "CommandsController.h"
#include "CopyPasteController.h"
#include <QVariant>
#include <QMenu>
#include <QMessageBox>
#include "IconHelper.h"

#define ITEM_ID 0, Qt::UserRole

#define MENU_ITEM_DELETE tr("Delete")
#define MENU_ITEM_COPY tr("Copy")
#define MENU_ITEM_PASTE tr("Paste")
#define MENU_ITEM_CREATE_SCREEN tr("Create screen")


HierarchyTreeWidget::HierarchyTreeWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HierarchyTreeWidget)
{
    ui->setupUi(this);
	
	connect(HierarchyTreeController::Instance(), SIGNAL(HierarchyTreeUpdated()), this, SLOT(OnTreeUpdated()));
	connect(HierarchyTreeController::Instance(),
			SIGNAL(SelectedControlNodesChanged(const HierarchyTreeController::SELECTEDCONTROLNODES &)),
			this,
			SLOT(OnSelectedControlNodesChanged(const HierarchyTreeController::SELECTEDCONTROLNODES &)));
	
	connect(ui->treeWidget, SIGNAL(ShowCustomMenu(const QPoint&)), this, SLOT(OnShowCustomMenu(const QPoint&)));
	internalSelectionChanged = false;
}

HierarchyTreeWidget::~HierarchyTreeWidget()
{
    delete ui;
}

void HierarchyTreeWidget::OnTreeUpdated()
{
	EXPANDEDITEMS expandedItems;
	EXPANDEDITEMS selectedItems;
	//save opened node
	TREEITEMS oldItems = GetAllItems();
	for (TREEITEMS::iterator iter = oldItems.begin(); iter != oldItems.end(); ++iter) {
		QTreeWidgetItem* item = iter->second;
		if (item->isExpanded())
		{
			QVariant data = item->data(ITEM_ID);
			expandedItems.insert(data.toInt());
		}
	}

	//save selected node
	
	for (TREEITEMS::iterator iter = oldItems.begin(); iter != oldItems.end(); ++iter) {
		QTreeWidgetItem* item = iter->second;
		if (item->isSelected())
		{
			QVariant data = item->data(ITEM_ID);
			selectedItems.insert(data.toInt());
		}
	}
	
	//reset tree
	ui->treeWidget->clear();
	
	const HierarchyTree& tree = HierarchyTreeController::Instance()->GetTree();
	const HierarchyTreeRootNode* rootNode = tree.GetRootNode();
	for (HierarchyTreeNode::HIERARCHYTREENODESLIST::const_iterator iter = rootNode->GetChildNodes().begin();
		 iter != rootNode->GetChildNodes().end();
		 ++iter)
	{
		//add platform node
		const HierarchyTreePlatformNode* platformNode = (const HierarchyTreePlatformNode*)(*iter);
		QTreeWidgetItem* platformItem = new QTreeWidgetItem();
		platformItem->setData(ITEM_ID, platformNode->GetId());
		platformItem->setText(0, platformNode->GetName());
		platformItem->setIcon(0, QIcon(":/icons/079.png"));
		ui->treeWidget->insertTopLevelItem(ui->treeWidget->topLevelItemCount(), platformItem);
		
		for (HierarchyTreeNode::HIERARCHYTREENODESLIST::const_iterator iter = platformNode->GetChildNodes().begin();
			 iter != platformNode->GetChildNodes().end();
			 ++iter)
		{
			const HierarchyTreeScreenNode* screenNode = dynamic_cast<const HierarchyTreeScreenNode*>(*iter);
			DVASSERT(screenNode);

			QTreeWidgetItem* screenItem = new QTreeWidgetItem();
			screenItem->setData(ITEM_ID, screenNode->GetId());

			// Check whether this screen was changed.
			QString screenItemText = screenNode->GetName();
			if (screenNode->GetUnsavedChanges() != 0)
			{
				screenItemText += " *";
			}
			screenItem->setText(0, screenItemText);

			if (dynamic_cast<const HierarchyTreeAggregatorNode*>(screenNode))
				screenItem->setIcon(0, QIcon(":/icons/170.png"));
			else
				screenItem->setIcon(0, QIcon(":/icons/068.png"));
			platformItem->insertChild(platformItem->childCount(), screenItem);
			
			AddControlItem(screenItem, selectedItems, expandedItems, screenNode->GetChildNodes());
			
			if (expandedItems.find(screenNode->GetId()) != expandedItems.end())
				screenItem->setExpanded(true);

			if (selectedItems.find(screenNode->GetId()) != selectedItems.end())
			{
				DAVA::Logger().Debug("***Screen selected!***");
				screenItem->setSelected(true);
			}
		}
		
		if (expandedItems.find(platformNode->GetId()) != expandedItems.end())
			platformItem->setExpanded(true);

		if (selectedItems.find(platformNode->GetId()) != selectedItems.end())
		{
			DAVA::Logger().Debug("***Platform selected!***");
			platformItem->setSelected(true);
		}
	}
}

void HierarchyTreeWidget::AddControlItem(QTreeWidgetItem* parent, const EXPANDEDITEMS& selectedItems, const EXPANDEDITEMS& expandedItems, const HierarchyTreeNode::HIERARCHYTREENODESLIST& items)
{
	for (HierarchyTreeNode::HIERARCHYTREENODESLIST::const_iterator iter = items.begin();
		 iter != items.end();
		 ++iter)
	{
		const HierarchyTreeControlNode* controlNode = (const HierarchyTreeControlNode*)(*iter);
		
		QTreeWidgetItem* controlItem = new QTreeWidgetItem();
		controlItem->setData(ITEM_ID, controlNode->GetId());
		controlItem->setText(0, controlNode->GetName());

		DecorateWithIcon(controlItem, controlNode->GetUIObject());

		parent->insertChild(parent->childCount(), controlItem);
		
		AddControlItem(controlItem, selectedItems, expandedItems, controlNode->GetChildNodes());
		
		if (expandedItems.find(controlNode->GetId()) != expandedItems.end())
			controlItem->setExpanded(true);
		if (selectedItems.find(controlNode->GetId()) != selectedItems.end())
		{
			QVariant data = controlItem->data(ITEM_ID);
			HierarchyTreeNode::HIERARCHYTREENODEID id = data.toInt();
			DAVA::Logger().Debug("***Control %d selected!***", id);
			controlItem->setSelected(true);
		}
	}
}

void HierarchyTreeWidget::DecorateWithIcon(QTreeWidgetItem *item, DAVA::UIControl *uiControl)
{
	if (!item || !uiControl)
	{
		return;
	}

	QString iconPath = IconHelper::GetIconPathForUIControl(uiControl);
	item->setIcon(0, QIcon(iconPath));
}

void HierarchyTreeWidget::on_treeWidget_itemSelectionChanged()
{
	QTreeWidgetItem* selectedItem = ui->treeWidget->currentItem();
	if (!selectedItem)
	{
		//DAVA::Logger().Debug("***HierarchyTreeWidget::on_treeWidget_itemSelectionChanged()  !selectedItem return ***");
		return;
	}

	QVariant data = selectedItem->data(ITEM_ID);
	HierarchyTreeNode::HIERARCHYTREENODEID id = data.toInt();
	//DAVA::Logger().Debug("***HierarchyTreeWidget::on_treeWidget_itemSelectionChanged()  %d ***", id);

	if (internalSelectionChanged)
	{
		//DAVA::Logger().Debug("***HierarchyTreeWidget::on_treeWidget_itemSelectionChanged()  internalSelectionChanged ! return ***");
		return;
	}

	
	HierarchyTreeNode* baseNode = HierarchyTreeController::Instance()->GetTree().GetNode(id);
	HierarchyTreePlatformNode* selectedPlatform = dynamic_cast<HierarchyTreePlatformNode* >(baseNode);
	HierarchyTreeScreenNode* selectedScreen =  dynamic_cast<HierarchyTreeScreenNode* >(baseNode);
	HierarchyTreeControlNode* selectedControl = dynamic_cast<HierarchyTreeControlNode* >(baseNode);
	
	if (!selectedPlatform && !selectedScreen && !selectedControl)
		return;
	
	internalSelectionChanged = true;
	
	//only platform or screen node can be seleted
	if (selectedPlatform || selectedScreen)
	{
		ResetSelection();
		HierarchyTreeController::Instance()->ResetSelectedControl();
		DAVA::Logger().Debug("***Element %d selected!!!!***", id);
		selectedItem->setSelected(true);
	}
	
	if (selectedControl)
	{
		selectedScreen = selectedControl->GetScreenNode();
		selectedPlatform = selectedScreen->GetPlatform();
	}
	else if (selectedScreen)
	{
		selectedPlatform = selectedScreen->GetPlatform();
	}
		
	HierarchyTreeController::Instance()->UpdateSelection(selectedPlatform, selectedScreen);
	HierarchyTreeController::Instance()->UpdateSelection(baseNode);
	
	internalSelectionChanged = false;
	
	if (selectedControl)
	{
		if (ui->treeWidget->selectedItems().size() == 1)
			HierarchyTreeController::Instance()->ResetSelectedControl();
		HierarchyTreeController::Instance()->SelectControl(selectedControl);
	}
}

void HierarchyTreeWidget::ResetSelection()
{
	QList<QTreeWidgetItem*> selectedList = ui->treeWidget->selectedItems();
	for (QList<QTreeWidgetItem*>::iterator iter = selectedList.begin(); iter != selectedList.end(); ++iter) {
		QTreeWidgetItem* item = (*iter);
		item->setSelected(false);
	}
}

void HierarchyTreeWidget::OnSelectedControlNodesChanged(const HierarchyTreeController::SELECTEDCONTROLNODES &selectedControls)
{
	if (internalSelectionChanged)
		return;
	
	internalSelectionChanged = true;
	ResetSelection();
	
	TREEITEMS items = GetAllItems();
	for (HierarchyTreeController::SELECTEDCONTROLNODES::const_iterator iter = selectedControls.begin();
		 iter != selectedControls.end();
		 ++iter)
	{
		const HierarchyTreeControlNode* node = (*iter);
		TREEITEMS::iterator itemIter = items.find(node->GetId());
		if (itemIter != items.end())
		{
            QTreeWidgetItem* item = itemIter->second;

			QVariant data = item->data(ITEM_ID);
			HierarchyTreeNode::HIERARCHYTREENODEID id = data.toInt();
			DAVA::Logger().Debug("***Item %d selected! (HierarchyTreeWidget::OnSelectedControlNodesChanged)***", id);

			item->setSelected(true);
			
			// Force show selected item
            QTreeWidgetItem* parentItem = item->parent();
            while (parentItem)
            {
                parentItem->setExpanded(true);
                parentItem = parentItem->parent();
            }
		}
	}

	internalSelectionChanged = false;
}

HierarchyTreeWidget::TREEITEMS HierarchyTreeWidget::GetAllItems()
{
	TREEITEMS items;
	
	for (int i = 0; i < ui->treeWidget->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* item = ui->treeWidget->topLevelItem(i);
		if (!item)
			continue;
		
		QVariant data = item->data(ITEM_ID);
		items[data.toInt()] = item;
		
		if (item->childCount())
			GetChildItems(item, items);
	}
	
	return items;
}

void HierarchyTreeWidget::GetChildItems(const QTreeWidgetItem* parent, TREEITEMS &items)
{
	for(int i = 0; i < parent->childCount(); i++)
	{
		QTreeWidgetItem* item = parent->child(i);

		QVariant data = item->data(ITEM_ID);
		items[data.toInt()] = item;
		
		if (item->childCount())
			GetChildItems(item, items);
	}
}

void HierarchyTreeWidget::OnShowCustomMenu(const QPoint& pos)
{
	QList<QTreeWidgetItem*> items = ui->treeWidget->selectedItems();
	if (!items.size())
		return;

	QTreeWidgetItem* item = items.at(0);
	QVariant data = item->data(ITEM_ID);
	HierarchyTreeNode::HIERARCHYTREENODEID id = data.toInt();
	
	HierarchyTreeNode* baseNode = HierarchyTreeController::Instance()->GetTree().GetNode(id);
	HierarchyTreePlatformNode* selectedPlatform = dynamic_cast<HierarchyTreePlatformNode* >(baseNode);
	HierarchyTreeScreenNode* selectedScreen =  dynamic_cast<HierarchyTreeScreenNode* >(baseNode);
	HierarchyTreeControlNode* selectedControl = dynamic_cast<HierarchyTreeControlNode* >(baseNode);

	QMenu menu;
	if (selectedControl || selectedPlatform || selectedScreen)
	{
		QAction* deleteControlAction = new QAction(MENU_ITEM_DELETE, &menu);
		connect(deleteControlAction, SIGNAL(triggered()), this, SLOT(OnDeleteControlAction()));
		menu.addAction(deleteControlAction);
		
		QAction* copyControlAction = new QAction(MENU_ITEM_COPY, &menu);
		connect(copyControlAction, SIGNAL(triggered()), this, SLOT(OnCopyAction()));
		menu.addAction(copyControlAction);
	}
	if (selectedPlatform)
	{
		QAction* createScreenAction = new QAction(MENU_ITEM_CREATE_SCREEN, &menu);
		connect(createScreenAction, SIGNAL(triggered()), this, SLOT(OnCreateScreenAction()));
		menu.addAction(createScreenAction);
		
		if (CopyPasteController::Instance()->GetCopyType() == CopyPasteController::CopyTypeScreen)
		{
			QAction* pasteScreenAction = new QAction(MENU_ITEM_PASTE, &menu);
			connect(pasteScreenAction, SIGNAL(triggered()), this, SLOT(OnPasteAction()));
			menu.addAction(pasteScreenAction);
		}
	}
	else if (selectedScreen || selectedControl)
	{
		if (CopyPasteController::Instance()->GetCopyType() == CopyPasteController::CopyTypeControl)
		{
			QAction* pasteScreenAction = new QAction(MENU_ITEM_PASTE, &menu);
			connect(pasteScreenAction, SIGNAL(triggered()), this, SLOT(OnPasteAction()));
			menu.addAction(pasteScreenAction);
		}
	}
	menu.exec(pos);
}

void HierarchyTreeWidget::OnDeleteControlAction()
{
	QList<QTreeWidgetItem*> items = ui->treeWidget->selectedItems();
	if (!items.size())
		return;

	bool needConfirm = false;
	HierarchyTreeNode::HIERARCHYTREENODESLIST nodes;
	for (QList<QTreeWidgetItem*>::iterator iter = items.begin(); iter != items.end(); ++iter)
	{
		QTreeWidgetItem* item = (*iter);
		QVariant data = item->data(ITEM_ID);
		HierarchyTreeNode::HIERARCHYTREENODEID id = data.toInt();
		HierarchyTreeNode* node = HierarchyTreeController::Instance()->GetTree().GetNode(id);
		
		HierarchyTreeAggregatorNode* aggregatorNode = dynamic_cast<HierarchyTreeAggregatorNode*>(node);
		if (aggregatorNode)
		{
			const HierarchyTreeAggregatorNode::CHILDS& childs = aggregatorNode->GetChilds();
			needConfirm |= (childs.size() > 0);
			for (HierarchyTreeAggregatorNode::CHILDS::const_iterator iter = childs.begin(); iter != childs.end(); ++iter)
			{
				nodes.push_back((*iter));
			}
		}
		
		nodes.push_back(node);
	}
	
	if (needConfirm)
	{
		if (QMessageBox::No == QMessageBox::information(this,
								 "",
								 "Selected aggregator control has child controls. Do you want delete aggregator with all child controls?",
								 QMessageBox::Yes | QMessageBox::No))
			return;
	}
	
	DeleteSelectedNodeCommand* cmd = new DeleteSelectedNodeCommand(nodes);
	CommandsController::Instance()->ExecuteCommand(cmd);
	SafeRelease(cmd);
}

void HierarchyTreeWidget::OnCreateScreenAction()
{
	QList<QTreeWidgetItem*> items = ui->treeWidget->selectedItems();
	if (!items.size())
		return;
	QTreeWidgetItem* item = items.at(0);
	QVariant data = item->data(ITEM_ID);
	HierarchyTreeNode::HIERARCHYTREENODEID id = data.toInt();
	emit CreateNewScreen(id);
}

void HierarchyTreeWidget::OnCopyAction()
{
	QList<QTreeWidgetItem*> items = ui->treeWidget->selectedItems();
	if (!items.size())
		return;

	QTreeWidgetItem* item = items.at(0);
	QVariant data = item->data(ITEM_ID);
	HierarchyTreeNode::HIERARCHYTREENODEID id = data.toInt();
	
	HierarchyTreeNode* baseNode = HierarchyTreeController::Instance()->GetTree().GetNode(id);
	HierarchyTreePlatformNode* selectedPlatform = dynamic_cast<HierarchyTreePlatformNode* >(baseNode);
	HierarchyTreeScreenNode* selectedScreen =  dynamic_cast<HierarchyTreeScreenNode* >(baseNode);
	HierarchyTreeControlNode* selectedControl = dynamic_cast<HierarchyTreeControlNode* >(baseNode);
	
	if (selectedControl)
	{
		CopyPasteController::Instance()->CopyControls(HierarchyTreeController::Instance()->GetActiveControlNodes());
	}
	else if (selectedScreen)
	{
		HierarchyTreeNode::HIERARCHYTREENODESLIST list;
		list.push_back(selectedScreen);
		CopyPasteController::Instance()->Copy(list);
	}
	else if (selectedPlatform)
	{
		HierarchyTreeNode::HIERARCHYTREENODESLIST list;
		list.push_back(selectedPlatform);
		CopyPasteController::Instance()->Copy(list);
		CopyPasteController::Instance()->Paste(selectedPlatform->GetRoot());
	}
}

void HierarchyTreeWidget::OnPasteAction()
{
	QList<QTreeWidgetItem*> items = ui->treeWidget->selectedItems();
	if (!items.size())
		return;
	
	QTreeWidgetItem* item = items.at(0);
	QVariant data = item->data(ITEM_ID);
	HierarchyTreeNode::HIERARCHYTREENODEID id = data.toInt();
	
	HierarchyTreeNode* baseNode = HierarchyTreeController::Instance()->GetTree().GetNode(id);

	CopyPasteController::Instance()->Paste(baseNode);
}
