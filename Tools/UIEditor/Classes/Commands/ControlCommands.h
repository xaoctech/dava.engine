//
//  ControlsMoveCommand.h
//  UIEditor
//
//  Created by adebt on 10/29/12.
//
//

#ifndef __UIEditor__ControlsMoveCommand__
#define __UIEditor__ControlsMoveCommand__

#include "BaseCommand.h"
#include "HierarchyTreeController.h"

using namespace DAVA;

class ControlsMoveCommand: public BaseCommand
{
public:
	ControlsMoveCommand(const HierarchyTreeController::SELECTEDCONTROLNODES& controls, const Vector2& delta);
	virtual void Execute();

private:
	HierarchyTreeController::SELECTEDCONTROLNODES controls;
	Vector2 delta;
};

class ControlResizeCommand: public BaseCommand
{
public:
	ControlResizeCommand(HierarchyTreeNode::HIERARCHYTREENODEID nodeId, const Rect& originalRect, const Rect& newRect);
	virtual void Execute();
	
private:
	HierarchyTreeNode::HIERARCHYTREENODEID nodeId;
	Rect originalRect;
	Rect newRect;
};


#endif /* defined(__UIEditor__ControlsMoveCommand__) */
