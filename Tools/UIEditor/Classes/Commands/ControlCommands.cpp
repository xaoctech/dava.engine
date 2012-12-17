//
//  ControlsMoveCommand.cpp
//  UIEditor
//
//  Created by adebt on 10/29/12.
//
//

#include "ControlCommands.h"
#include "CommandsController.h"

using namespace DAVA;

ControlsMoveCommand::ControlsMoveCommand(const HierarchyTreeController::SELECTEDCONTROLNODES& controls, const Vector2& delta)
{
	this->controls = controls;
	this->delta = delta;
}

void ControlsMoveCommand::Execute()
{
    for (HierarchyTreeController::SELECTEDCONTROLNODES::iterator iter = this->controls.begin();
         iter != this->controls.end(); iter ++)
    {
        BaseMetadata* baseMetadata = GetMetadataForTreeNode((*iter)->GetId());

        // This command is NOT state-aware and contains one and only param.
        baseMetadata->SetActiveParamID(0);
        baseMetadata->ApplyMove(this->delta);

        SAFE_DELETE(baseMetadata);
    }
    
    // Notify the Grid some properties were changed.
    CommandsController::Instance()->EmitUpdatePropertyValues();
}

ControlResizeCommand::ControlResizeCommand(HierarchyTreeNode::HIERARCHYTREENODEID nodeId, const Rect& originalRect, const Rect& newRect)
{
	this->nodeId = nodeId;
	this->originalRect = originalRect;
	this->newRect = newRect;
}

void ControlResizeCommand::Execute()
{
    BaseMetadata* baseMetadata = GetMetadataForTreeNode(nodeId);
    
    // This command is NOT state-aware and contains one and only param.
    baseMetadata->SetActiveParamID(0);
    baseMetadata->ApplyResize(originalRect, newRect);
    
    SAFE_DELETE(baseMetadata);
}
