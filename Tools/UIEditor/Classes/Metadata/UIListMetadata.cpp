//
//  UIListMetadata.cpp
//  UIEditor
//
//  Created by Yuri Coder on 3/11/13.
//
//

#include "UIListMetadata.h"
#include "EditorListDelegate.h"

namespace DAVA {

UIListMetadata::UIListMetadata(QObject* parent) :
	UIControlMetadata(parent)
{
}

UIList* UIListMetadata::GetActiveUIList() const
{
	return dynamic_cast<UIList*>(GetActiveUIControl());
}

void UIListMetadata::InitializeControl(const String& controlName, const Vector2& position)
{
	UIControlMetadata::InitializeControl(controlName, position);
	
	int paramsCount = this->GetParamsCount();
    for (BaseMetadataParams::METADATAPARAMID i = 0; i < paramsCount; i ++)
    {
		// Initialize UIList
        UIList* list = dynamic_cast<UIList*>(this->treeNodeParams[i].GetUIControl());
		if (list)
		{
			EditorListDelegate *editorList = new EditorListDelegate(list->GetRect());
			list->SetDelegate(editorList);
			list->GetBackground()->SetDrawType(UIControlBackground::DRAW_SCALE_TO_RECT);
		}
    }	
}

void UIListMetadata::UpdateExtraData(HierarchyTreeNodeExtraData& extraData, eExtraDataUpdateStyle updateStyle)
{
	UIControlMetadata::UpdateExtraData(extraData, updateStyle);
}

int UIListMetadata::GetAggregatorID()
{
    if (!VerifyActiveParamID())
    	return HierarchyTreeNode::HIERARCHYTREENODEID_EMPTY;

	EditorListDelegate *editorList = (EditorListDelegate *)GetActiveUIList()->GetDelegate();	
	if (!editorList)
		return HierarchyTreeNode::HIERARCHYTREENODEID_EMPTY;

	return editorList->GetAggregatorID();
}

void UIListMetadata::SetAggregatorID(int value)
{
	if (!VerifyActiveParamID())
    	return;

	EditorListDelegate *editorList = (EditorListDelegate *)GetActiveUIList()->GetDelegate();
	if (!editorList)
		return;

	editorList->SetAggregatorID(value);
	GetActiveUIList()->Refresh();
}

void UIListMetadata::SetActiveControlRect(const Rect& rect)
{
	UIControlMetadata::SetActiveControlRect(rect);
	// Get delegate for current list
	EditorListDelegate *editorList = (EditorListDelegate *)GetActiveUIList()->GetDelegate();	
	if (!editorList)
		return;
	// Update cell width
	float32 width = rect.dx;
	// Calculate new height of list cell by dividing list height on elements count
	float32 height = rect.dy / editorList->ElementsCount(GetActiveUIList());
	// Update cell size with new height and width
	editorList->SetCellSize(Vector2(width, height));
	// Refresh list - and recreate cells
	GetActiveUIList()->Refresh();
}

};