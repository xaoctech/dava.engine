#include "Classes/Selection/Selection.h"
#include "Classes/Selection/SelectionData.h"
#include "Classes/Application/REGlobal.h"

#include "TArc/DataProcessing/DataContext.h"

namespace Selection
{
const SelectableGroup& GetSelection()
{
    SelectionData* selectionData = REGlobal::GetActiveDataNode<SelectionData>();
    if (selectionData != nullptr)
    {
        return selectionData->GetSelection();
    }

    static SelectableGroup emptyGroup;
    return emptyGroup;
}

void CancelSelection()
{
    SelectionData* selectionData = REGlobal::GetActiveDataNode<SelectionData>();
    if (selectionData != nullptr)
    {
        selectionData->CancelSelection();
    }
}

void SetSelection(SelectableGroup& newSelection)
{
    SelectionData* selectionData = REGlobal::GetActiveDataNode<SelectionData>();
    if (selectionData != nullptr)
    {
        selectionData->SetSelection(newSelection);
    }
}

DAVA::Entity* GetSelectableEntity(DAVA::Entity* selectionCandidate)
{
    DAVA::Entity* parent = selectionCandidate;
    while (nullptr != parent)
    {
        if (parent->GetSolid())
        {
            selectionCandidate = parent;
        }
        parent = parent->GetParent();
    }
    return selectionCandidate;
}

bool IsEntitySelectable(DAVA::Entity* selectionCandidate)
{
    SelectionData* selectionData = REGlobal::GetActiveDataNode<SelectionData>();
    if (selectionData != nullptr)
    {
        return selectionData->IsEntitySelectable(selectionCandidate);
    }

    return false;
}

void ResetSelectionComponentMask()
{
    SelectionData* selectionData = REGlobal::GetActiveDataNode<SelectionData>();
    if (selectionData != nullptr)
    {
        selectionData->ResetSelectionComponentMask();
    }
}

void SetSelectionComponentMask(const DAVA::ComponentMask& mask)
{
    SelectionData* selectionData = REGlobal::GetActiveDataNode<SelectionData>();
    if (selectionData != nullptr)
    {
        selectionData->SetSelectionComponentMask(mask);
    }
}

DAVA::ComponentMask GetSelectionComponentMask()
{
    SelectionData* selectionData = REGlobal::GetActiveDataNode<SelectionData>();
    if (selectionData != nullptr)
    {
        return selectionData->GetSelectionComponentMask();
    }

    return DAVA::ComponentMask();
}

bool Lock()
{
    SelectionData* selectionData = REGlobal::GetActiveDataNode<SelectionData>();
    if (selectionData != nullptr)
    {
        return selectionData->Lock();
    }
    return false;
}

void Unlock()
{
    SelectionData* selectionData = REGlobal::GetActiveDataNode<SelectionData>();
    if (selectionData != nullptr)
    {
        selectionData->Unlock();
    }
}
}
