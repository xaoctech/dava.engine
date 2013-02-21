#include "Classes/UI/hierarchytreecontrol.h"
#include <QAction>
#include <QMenu>
#include <QContextMenuEvent>
#include "HierarchyTreeController.h"
#include "ItemsCommand.h"
#include "CommandsController.h"
#include "CopyPasteController.h"

#define TREE_MIME_DATA 0
#define ITEM_ID 0, 1

HierarchyTreeControlMimeData::HierarchyTreeControlMimeData(const QList<QTreeWidgetItem*> items)
{
	for (QList<QTreeWidgetItem*>::const_iterator iter = items.begin(); iter != items.end(); ++iter)
	{
		QTreeWidgetItem* item = (*iter);
		QVariant data = item->data(ITEM_ID);
		HierarchyTreeNode::HIERARCHYTREENODEID id = data.toInt();
		this->items.insert(id);
	}
}

HierarchyTreeControlMimeData::~HierarchyTreeControlMimeData()
{
	
}

bool HierarchyTreeControlMimeData::IsDropEnable(const HierarchyTreeNode *parentItem) const
{
	if (items.find(parentItem->GetId()) != items.end())	//ignore self insert
	{
		return false;
	}
	
	const HierarchyTreePlatformNode* parentPlatform = dynamic_cast<const HierarchyTreePlatformNode*>(parentItem);
	const HierarchyTreeScreenNode* parentScreen = dynamic_cast<const HierarchyTreeScreenNode*>(parentItem);
	const HierarchyTreeControlNode* parentControl = dynamic_cast<const HierarchyTreeControlNode*>(parentItem);
	
	for (HierarchyTreeNode::HIERARCHYTREENODESIDLIST::const_iterator iter = items.begin(); iter != items.end(); ++iter)
	{
		HierarchyTreeNode::HIERARCHYTREENODEID id = (*iter);
		
		const HierarchyTreeNode* node = HierarchyTreeController::Instance()->GetTree().GetNode(id);
		if (parentItem == node) //skip self drop
			return false;
	
		if (node->IsHasChild(parentItem))
			return false;
		
		if (parentPlatform)
		{
			//all items must be screen
			const HierarchyTreeScreenNode* screen = dynamic_cast<const HierarchyTreeScreenNode*>(node);
			if (!screen)
				return false;
		}
		else  if (parentScreen || parentControl)
		{
			//all items must be control
			const HierarchyTreeControlNode* control = dynamic_cast<const HierarchyTreeControlNode*>(node);
			if (!control)
				return false;
		}
	}
	
	return true;
}

HierarchyTreeNode::HIERARCHYTREENODESIDLIST HierarchyTreeControlMimeData::GetItems() const
{
	return items;
}

HierarchyTreeControl::HierarchyTreeControl(QWidget *parent) :
    QTreeWidget(parent)
{
}

void HierarchyTreeControl::contextMenuEvent(QContextMenuEvent * event)
{
	emit ShowCustomMenu(event->globalPos());
}

QMimeData* HierarchyTreeControl::mimeData(const QList<QTreeWidgetItem*> items) const
{
	QMimeData* data = QTreeWidget::mimeData(items);
	data->setUserData(TREE_MIME_DATA, new HierarchyTreeControlMimeData(items));
	return data;
}

void HierarchyTreeControl::dropEvent(QDropEvent *event)
{
	//Get new parent item position at list
	QTreeWidgetItem* item = itemAt(event->pos());
	if (!item)
		return;
	//Get parent item tree node
	QVariant data = item->data(ITEM_ID);
	HierarchyTreeNode::HIERARCHYTREENODEID id = data.toInt();
	HierarchyTreeNode* parentNode = HierarchyTreeController::Instance()->GetTree().GetNode(id);
	if (!parentNode)
		return;
	
	const HierarchyTreeControlMimeData* mimeData = dynamic_cast<const HierarchyTreeControlMimeData* >(event->mimeData()->userData(TREE_MIME_DATA));
	if (!mimeData)
		return;
	
	if (!mimeData->IsDropEnable(parentNode))
		return;
		
	//Copy current selected item(s) if ctrl key is pressed during drag
	if (event->keyboardModifiers() == Qt::ControlModifier)
	{
		CopyPasteController::Instance()->CopyControls(HierarchyTreeController::Instance()->GetActiveControlNodes());
		CopyPasteController::Instance()->Paste(parentNode);
	}
	else //Otherwise move item(s)
	{
		HierarchyTreeNode::HIERARCHYTREENODESIDLIST items = mimeData->GetItems();
		ChangeNodeHeirarchy* cmd = new ChangeNodeHeirarchy(parentNode->GetId(), items);
		CommandsController::Instance()->ExecuteCommand(cmd);
		SafeRelease(cmd);
	}
}

void HierarchyTreeControl::dragEnterEvent(QDragEnterEvent *event)
{
	if (!event->mimeData())
		return;
	
	const HierarchyTreeControlMimeData* mimeData = dynamic_cast<const HierarchyTreeControlMimeData* >(event->mimeData()->userData(TREE_MIME_DATA));
	if (!mimeData)
		return;

	event->accept();
}

void HierarchyTreeControl::dragMoveEvent(QDragMoveEvent *event)
{
	event->ignore();
	
	if (!event->mimeData())
		return;
		
	QTreeWidgetItem* item = itemAt(event->pos());
	if (!item)
		return;
	
	QVariant data = item->data(ITEM_ID);
	HierarchyTreeNode::HIERARCHYTREENODEID id = data.toInt();
	HierarchyTreeNode* node = HierarchyTreeController::Instance()->GetTree().GetNode(id);
	if (!node)
		return;
	
	const HierarchyTreeControlMimeData* mimeData = dynamic_cast<const HierarchyTreeControlMimeData* >(event->mimeData()->userData(TREE_MIME_DATA));
	if (!mimeData)
		return;

	if (mimeData->IsDropEnable(node))
	{
		QTreeWidget::dragMoveEvent(event);
		event->accept();
	}
}