//
//  PasteCommand.cpp
//  UIEditor
//
//  Created by adebt on 11/5/12.
//
//

#include "PasteCommand.h"
#include "HierarchyTreeAggregatorControlNode.h"

#define COPY_DELTA Vector2(5, 5)

PasteCommand::PasteCommand(HierarchyTreeNode* parentNode, CopyPasteController::CopyType copyType, const HierarchyTreeNode::HIERARCHYTREENODESLIST* items)
{
	this->parentNode = parentNode;
	this->copyType = copyType;
	this->items = items;
	this->newItems = NULL;
}

PasteCommand::~PasteCommand()
{
	CleanupPastedItems();
	//delete this->items;
}


void PasteCommand::Execute()
{
	HierarchyTreeController::Instance()->ResetSelectedControl();
	
	int count = 0;
	
	if (this->newItems)
	{
		// We are performing Rollback after Execute.
		ReturnPastedControlsToScene();
		return;
	}

	// This is the first Execute - remember the items to be pasted.
	this->newItems = new HierarchyTreeNode::HIERARCHYTREENODESLIST();

	switch (copyType) {
		case CopyPasteController::CopyTypeControl:
		{
			if (parentNode)
			{
                count += PasteControls(newItems, parentNode);
				for (HierarchyTreeNode::HIERARCHYTREENODESLIST::iterator iter = newItems->begin();
					 iter != newItems->end();
					 ++iter)
				{
					HierarchyTreeNode* node = (*iter);
					HierarchyTreeControlNode* controlNode = dynamic_cast<HierarchyTreeControlNode* >(node);
					if (!controlNode)
						continue;
					
					parentNode->AddTreeNode(controlNode);
                    //Check type of parent control
                    //For screen control there is another sequence
                    HierarchyTreeScreenNode* parentScreen = dynamic_cast<HierarchyTreeScreenNode*>(parentNode);
                    HierarchyTreeControlNode* parentControl = dynamic_cast<HierarchyTreeControlNode *>(parentNode);
                    if (parentScreen)
                    {
                        parentScreen->GetScreen()->AddControl(controlNode->GetUIObject());
                    }
                    else if (parentControl)
                    {
                        parentControl->GetUIObject()->AddControl(controlNode->GetUIObject()); 
                    }
					
					HierarchyTreeController::Instance()->ChangeItemSelection(controlNode);
				}
			}
		}break;
		case CopyPasteController::CopyTypeScreen:
		{
			HierarchyTreePlatformNode* parentPlatform = dynamic_cast<HierarchyTreePlatformNode*>(parentNode);
			if (parentPlatform)
			{
				count += PasteScreens(newItems, parentPlatform);
				for (HierarchyTreeNode::HIERARCHYTREENODESLIST::iterator iter = newItems->begin();
					 iter != newItems->end();
					 ++iter)
				{
					HierarchyTreeNode* node = (*iter);
					HierarchyTreeScreenNode* screenNode = dynamic_cast<HierarchyTreeScreenNode* >(node);
					if (!screenNode)
						continue;
					
					parentPlatform->AddTreeNode(screenNode);
				}
			}
		}break;
		case CopyPasteController::CopyTypePlatform:
		{
			HierarchyTreeRootNode* parentRoot = dynamic_cast<HierarchyTreeRootNode*>(parentNode);
			if (parentRoot)
			{
				count += PastePlatforms(newItems, parentRoot);
				for (HierarchyTreeNode::HIERARCHYTREENODESLIST::iterator iter = newItems->begin();
					 iter != newItems->end();
					 ++iter)
				{
					HierarchyTreeNode* node = (*iter);
					HierarchyTreePlatformNode* platformNode = dynamic_cast<HierarchyTreePlatformNode* >(node);
					if (!platformNode)
						continue;
					
					parentRoot->AddTreeNode(platformNode);
				}
			}
		}break;
			
		default:
			break;
	}
	
	HierarchyTreeController::Instance()->EmitHierarchyTreeUpdated();
}

void PasteCommand::Rollback()
{
	if (!newItems)
	{
		return;
	}
	
	// Cleanup the new items, if any.
	for (HierarchyTreeNode::HIERARCHYTREENODESLIST::iterator iter = this->newItems->begin();
		 iter != this->newItems->end(); ++iter)
	{
		// Remove the controls added and appropriate nodes. Don't delete them from
		// memory - we might return them in Rollback.
		HierarchyTreeNode* curNode = (*iter);
		curNode->PrepareRemoveFromSceneInformation();
		HierarchyTreeController::Instance()->DeleteNode(curNode->GetId(), false, true);
	}

	HierarchyTreeController::Instance()->EmitHierarchyTreeUpdated();
}

int PasteCommand::PasteControls(HierarchyTreeNode::HIERARCHYTREENODESLIST* newControls, HierarchyTreeNode *parent)
{
	int count = 0;
	for (HierarchyTreeNode::HIERARCHYTREENODESCONSTITER iter = items->begin();
		 iter != items->end();
		 ++iter)
	{
		HierarchyTreeNode* node = (*iter);
		HierarchyTreeControlNode* control = dynamic_cast<HierarchyTreeControlNode*>(node);
		if (!control)
			continue;
		
		HierarchyTreeAggregatorControlNode* aggregatorControl = dynamic_cast<HierarchyTreeAggregatorControlNode*>(control);
		HierarchyTreeControlNode* copy = NULL;
		if (aggregatorControl)
			copy = new HierarchyTreeAggregatorControlNode(parent, aggregatorControl);
		else
			copy = new HierarchyTreeControlNode(parent, control);
		UpdateControlName(parent, copy, true);
		//copy->SetName(FormatCopyName(control->GetName(), parent));
		UIControl* clone = copy->GetUIObject();
		if (clone)
			clone->SetPosition(clone->GetPosition() + COPY_DELTA);
				
		count++;
		newControls->push_back(copy);
	}
	
	return count;
}

void PasteCommand::UpdateControlName(const HierarchyTreeNode* parent, HierarchyTreeNode* node, bool needCreateNewName) const
{
	QString name = node->GetName();
	if (needCreateNewName)
		name = FormatCopyName(node->GetName(), parent);
	node->SetName(name);
    
    // Also need to update the name of the UIControl - it is not copied.
    HierarchyTreeControlNode* controlNode = dynamic_cast<HierarchyTreeControlNode*>(node);
    if (controlNode && controlNode->GetUIObject())
    {
        controlNode->GetUIObject()->SetName(node->GetName().toStdString());
    }

	HierarchyTreeNode::HIERARCHYTREENODESLIST child = node->GetChildNodes();
	for (HierarchyTreeNode::HIERARCHYTREENODESLIST::iterator iter = child.begin();
		 iter != child.end();
		 ++iter)
	{
		HierarchyTreeNode* child = (*iter);
		UpdateControlName(parent, child, false);
	}
}

int PasteCommand::PasteScreens(HierarchyTreeNode::HIERARCHYTREENODESLIST* newScreens, HierarchyTreePlatformNode* parent)
{
	int count = 0;
	for (HierarchyTreeNode::HIERARCHYTREENODESCONSTITER iter = items->begin();
		 iter != items->end();
		 ++iter)
	{
		HierarchyTreeNode* node = (*iter);
		HierarchyTreeScreenNode* screen = dynamic_cast<HierarchyTreeScreenNode*>(node);
		if (!screen)
			continue;
		
		HierarchyTreeScreenNode* copy = new HierarchyTreeScreenNode(parent, screen);
        UpdateControlName(parent, copy, true);
		//copy->SetName(FormatCopyName(screen->GetName(), parent));
		
		count++;
		newScreens->push_back(copy);
	}
	
	return count;
}

int PasteCommand::PastePlatforms(HierarchyTreeNode::HIERARCHYTREENODESLIST* newScreens, HierarchyTreeRootNode* parent)
{
	int count = 0;
	for (HierarchyTreeNode::HIERARCHYTREENODESCONSTITER iter = items->begin();
		 iter != items->end();
		 ++iter)
	{
		HierarchyTreeNode* node = (*iter);
		HierarchyTreePlatformNode* platform = dynamic_cast<HierarchyTreePlatformNode*>(node);
		if (!platform)
			continue;
		
		HierarchyTreePlatformNode* copy = new HierarchyTreePlatformNode(parent, platform);
        UpdateControlName(parent, copy, true);
		//copy->SetName(FormatCopyName(platform->GetName(), parent));
		
		count++;
		newScreens->push_back(copy);
	}
	
	return count;
}

QString PasteCommand::FormatCopyName(QString baseName, const HierarchyTreeNode* parent) const
{
	QString name = baseName;
	QString numberName;
	const char* cName = name.toStdString().c_str();
	for (int i = name.length() - 1; i >= 0; --i)
	{
		char a = cName[i];
		if (a >= '0' && a <= '9')
			numberName = a + numberName;
		else
			break;
	}
	int id = numberName.toInt();
	baseName = name.left(name.length() - numberName.length());
	Logger::Debug(baseName.toStdString().c_str());
	
	const HierarchyTreeRootNode* parentRoot = dynamic_cast<const HierarchyTreeRootNode*>(parent);
	const HierarchyTreePlatformNode* parentPlatform = dynamic_cast<const HierarchyTreePlatformNode*>(parent);
	const HierarchyTreeScreenNode* parentScreen = dynamic_cast<const HierarchyTreeScreenNode*>(parent);
	if (!parentScreen)
	{
		const HierarchyTreeControlNode* parentControl = dynamic_cast<const HierarchyTreeControlNode*>(parent);
		if (parentControl)
		{
			parentScreen = parentControl->GetScreenNode();
		}
	}
	
	for (int i = 0; i < 1000; i++)
	{
		name = QString("%1%2").arg(baseName).arg(++id);
		Logger::Debug(name.toStdString().c_str());

		bool bFind = false;
		
		if (parentPlatform || parentRoot)
		{
			//chekc name only for one child level for screen and platform copy
			const HierarchyTreeNode::HIERARCHYTREENODESLIST& child = parent->GetChildNodes();
			for (HierarchyTreeNode::HIERARCHYTREENODESLIST::const_iterator iter = child.begin();
				 iter != child.end();
				 ++iter)
			{
				const HierarchyTreeNode* node = (*iter);
				if (node->GetName().compare(name) == 0)
					bFind = true;
			}
			if (!bFind)
				return name;
		}
		else
		{
			// copy control
			if (parentScreen)
			{
				if (!parentScreen->IsNameExist(name, parentScreen))
					return name;
			}
		}
	}
	return baseName;
}

void PasteCommand::ReturnPastedControlsToScene()
{
	if (!newItems)
	{
		return;
	}

	for (HierarchyTreeNode::HIERARCHYTREENODESLIST::iterator iter = this->newItems->begin();
		 iter != this->newItems->end(); ++iter)
	{
		// Return the removed node back to scene.
		HierarchyTreeNode* curNode = (*iter);
		HierarchyTreeController::Instance()->ReturnNodeToScene(curNode);
	}

	HierarchyTreeController::Instance()->EmitHierarchyTreeUpdated();
}

void PasteCommand::CleanupPastedItems()
{
	if (this->newItems == NULL)
	{
		return;
	}

	for (HierarchyTreeNode::HIERARCHYTREENODESLIST::iterator iter = this->newItems->begin();
		 iter != this->newItems->end(); ++iter)
	{
		SafeDelete(*iter);
	}
	
	SafeDelete(this->newItems);
}
