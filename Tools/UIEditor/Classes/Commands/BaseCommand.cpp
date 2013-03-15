//
//  BaseCommand.cpp
//  UIEditor
//
//  Created by Yuri Coder on 10/22/12.
//
//

#include "BaseCommand.h"
#include "MetadataFactory.h"

using namespace DAVA;

BaseCommand::BaseCommand()
{
 	activePlatform = activeScreen = HierarchyTreeNode::HIERARCHYTREENODEID_EMPTY;
	HierarchyTreeNode* platform = HierarchyTreeController::Instance()->GetActivePlatform();
	if (platform)
		activePlatform = platform->GetId();
	HierarchyTreeNode* screen = HierarchyTreeController::Instance()->GetActiveScreen();
	if (screen)
		activeScreen = screen->GetId();
}

BaseCommand::~BaseCommand()
{
}

BaseMetadata* BaseCommand::GetMetadataForTreeNode(HierarchyTreeNode::HIERARCHYTREENODEID treeNodeID)
{
    const HierarchyTreeNode* treeNode = HierarchyTreeController::Instance()->GetTree().GetNode(treeNodeID);
    if (treeNode == NULL)
    {
        Logger::Error("Tree Node is NULL for Tree Node ID %i", treeNodeID);
        return NULL;
    }
    
    UIControl* uiControl = NULL;
    const HierarchyTreeControlNode* controlNode = dynamic_cast<const HierarchyTreeControlNode*>(treeNode);
    if (controlNode)
    {
        uiControl = controlNode->GetUIObject();
    }
    
    BaseMetadata* baseMetadata = MetadataFactory::Instance()->GetMetadataForTreeNode(treeNode);
    if (baseMetadata == NULL)
    {
        Logger::Error("Unable to found Hierarchy Tree Node Metadata for node %i while executing Command!",
                      treeNodeID);
        return NULL;
    }
    
    METADATAPARAMSVECT params;
    params.push_back(BaseMetadataParams(treeNodeID, uiControl));
    baseMetadata->SetupParams(params);
    
    return baseMetadata;
}

void BaseCommand::ActivateCommandScreen()
{
	HierarchyTreePlatformNode* platform = dynamic_cast<HierarchyTreePlatformNode*>(HierarchyTreeController::Instance()->GetTree().GetNode(activePlatform));
	HierarchyTreeScreenNode* screen = dynamic_cast<HierarchyTreeScreenNode*>(HierarchyTreeController::Instance()->GetTree().GetNode(activeScreen));

	HierarchyTreeController::Instance()->UpdateSelection(platform, screen);
}