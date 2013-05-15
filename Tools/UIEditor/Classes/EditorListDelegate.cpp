//
//  EditorListDelegate.cpp
//  UIEditor
//
//  Created by Denis Bespalov on 4/5/13.
//
//

#include "EditorListDelegate.h"
#include "Utils/Utils.h"
#include "UI/UIAggregatorControl.h"

#include "HierarchyTreeAggregatorNode.h"
#include "HierarchyTreeAggregatorControlNode.h"
#include "HierarchyTreeController.h"
#include "EditorFontManager.h"

#define CELLS_COUNT 3
#define DEFAULT_AGGREGATOR_ID 0

namespace DAVA
{
	EditorListDelegate::EditorListDelegate(const Rect &rect, bool rectInAbsoluteCoordinates/* = FALSE*/)
	: UIControl(rect, rectInAbsoluteCoordinates)
	{
		cellSize = Vector2(rect.dx, (rect.dy / CELLS_COUNT));
		aggregatorID = DEFAULT_AGGREGATOR_ID;
	}
	
	EditorListDelegate::~EditorListDelegate()
	{
	}
	
	void EditorListDelegate::SetAggregatorID(int32 id)
	{
		aggregatorID = id;
	}
	
	int32 EditorListDelegate::GetAggregatorID()
	{
		return aggregatorID;
	}
	
	void EditorListDelegate::SetCellSize(const Vector2 &size)
	{
		cellSize = size;
	}
	
	// UIListDelegate implementation	
	int32 EditorListDelegate::ElementsCount(UIList *)
	{
		return CELLS_COUNT;
	}
	
	UIListCell *EditorListDelegate::CellAtIndex(UIList *forList, int32 index)
	{
    	// Try to get cell from the reusable cells store
    	UIListCell *cell = forList->GetReusableCell("Cell");
		if (!cell)
		{
			cell = new UIListCell(Rect(0.0f, 0.0f, cellSize.x, cellSize.y), "Cell");
		}
		else
		{
			cell->SetSize(Vector2(cellSize.x, cellSize.y));
		}
		
		// Get aggregator node
		HierarchyTreeNode *node = HierarchyTreeController::Instance()->GetTree().GetNode(aggregatorID);
		HierarchyTreeAggregatorNode *aggregatorNode = dynamic_cast<HierarchyTreeAggregatorNode*>(node);
	
		cell->RemoveAllControls();
		
		if (aggregatorNode)
		{
			UIAggregatorControl* control = new UIAggregatorControl();
			control->CopyDataFrom(aggregatorNode->GetScreen());
			cell->AddControl(control);
			
			control->SetRightAlignEnabled(true);
			control->SetLeftAlignEnabled(true);
			control->SetTopAlignEnabled(true);
			control->SetBottomAlignEnabled(true);
		}
		else
		{			
			cell->SetStateFont(UIControl::STATE_NORMAL, EditorFontManager::Instance()->GetDefaultFont());
			cell->SetStateText(UIControl::STATE_NORMAL, StringToWString(Format("Cell %d",index)));
			cell->SetSelected(true);
		}
   
    	return cell;
	}

	int32 EditorListDelegate::CellHeight(UIList *, int32)
	{
    	return cellSize.y;
	}
	
	void EditorListDelegate::SaveToYaml(UIList *forList, YamlNode *)
	{
		HierarchyTreeNode *node = HierarchyTreeController::Instance()->GetTree().GetNode(aggregatorID);
		HierarchyTreeAggregatorNode *aggregatorNode = dynamic_cast<HierarchyTreeAggregatorNode*>(node);
		if (aggregatorNode)
		{
			forList->SetAggregatorPath(aggregatorNode->GetPath());
		}
		else
		{
			forList->SetAggregatorPath(String());
		}
	}
};