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
	ApplyMove(this->delta);
    
    // Notify the Grid some properties were changed.
    CommandsController::Instance()->EmitUpdatePropertyValues();
}

void ControlsMoveCommand::Rollback()
{
	// Return the controls to the previous positions.
	Vector2 prevDelta = -1 * delta;
	ApplyMove(prevDelta);

    // Notify the Grid some properties were changed.
    CommandsController::Instance()->EmitUpdatePropertyValues();
}

void ControlsMoveCommand::ApplyMove(const Vector2& moveDelta)
{
	for (HierarchyTreeController::SELECTEDCONTROLNODES::iterator iter = this->controls.begin();
         iter != this->controls.end(); iter ++)
    {
        BaseMetadata* baseMetadata = GetMetadataForTreeNode((*iter)->GetId());
		
        // This command is NOT state-aware and contains one and only param.
        baseMetadata->SetActiveParamID(0);
        baseMetadata->ApplyMove(moveDelta);
		
        SAFE_DELETE(baseMetadata);
    }
}

ControlResizeCommand::ControlResizeCommand(HierarchyTreeNode::HIERARCHYTREENODEID nodeId, const Rect& originalRect, const Rect& newRect)
{
	this->nodeId = nodeId;
	this->originalRect = originalRect;
	this->newRect = newRect;
}

void ControlResizeCommand::Execute()
{
	ApplyResize(originalRect, newRect);
	
	// Notify the Grid some properties were changed.
    CommandsController::Instance()->EmitUpdatePropertyValues();
}

void ControlResizeCommand::Rollback()
{
	// Return the controls to the previous size.
	ApplyResize(newRect, originalRect);
	
    // Notify the Grid some properties were changed.
    CommandsController::Instance()->EmitUpdatePropertyValues();
}

void ControlResizeCommand::ApplyResize(const Rect& prevRect, const Rect& updatedRect)
{
	
    BaseMetadata* baseMetadata = GetMetadataForTreeNode(nodeId);

    // This command is NOT state-aware and contains one and only param.
    baseMetadata->SetActiveParamID(0);
    baseMetadata->ApplyResize(prevRect, updatedRect);
    
    SAFE_DELETE(baseMetadata);
}
