//
//  PropertiesGridController.cpp
//  UIEditor
//
//  Created by Yuri Coder on 10/18/12.
//
//

#include "PropertiesGridController.h"
#include "HierarchyTreeController.h"

#include "MetadataFactory.h"
#include "UIControlStateHelper.h"

using namespace DAVA;

PropertiesGridController::PropertiesGridController(QObject* parent) :
    QObject(parent)
{
    this->activeNode = NULL;
    this->activeUIControlState = UIControl::STATE_NORMAL;
    ConnectToSignals();
}

void PropertiesGridController::ConnectToSignals()
{
    connect(HierarchyTreeController::Instance(),
            SIGNAL(SelectedTreeItemChanged(const HierarchyTreeNode*)),
            this,
            SLOT(OnSelectedTreeItemChanged(const HierarchyTreeNode*)));

    connect(HierarchyTreeController::Instance(),
            SIGNAL(SelectedControlNodesChanged(const HierarchyTreeController::SELECTEDCONTROLNODES&)),
            this,
            SLOT(OnSelectedControlNodesChanged(const HierarchyTreeController::SELECTEDCONTROLNODES)));
}

PropertiesGridController::~PropertiesGridController()
{
}

const HierarchyTreeNode* PropertiesGridController::GetActiveTreeNode() const
{
    return this->activeNode;
}

const HierarchyTreeController::SELECTEDCONTROLNODES PropertiesGridController::GetActiveTreeNodesList() const
{
    return this->activeNodes;
}

UIControl::eControlState PropertiesGridController::GetActiveUIControlState() const
{
    return this->activeUIControlState;
}

void PropertiesGridController::SetActiveUIControlState(UIControl::eControlState newState)
{
    bool stateChanged = newState != this->activeUIControlState;
    this->activeUIControlState = newState;
    if (stateChanged)
    {
        emit SelectedUIControlStateChanged(this->activeUIControlState);
    }
}

void PropertiesGridController::OnSelectedControlNodesChanged(const HierarchyTreeController::SELECTEDCONTROLNODES
                                                             &selectedNodes)
{
    if (selectedNodes.size() == 0)
    {
        HandleNothingSelected();
        return;
    }
    
    UpdatePropertiesForUIControlNodeList(selectedNodes);
}

void PropertiesGridController::OnSelectedTreeItemChanged(const HierarchyTreeNode* selectedNode)
{
    UpdatePropertiesForSingleNode(selectedNode);
}

void PropertiesGridController::UpdatePropertiesForSingleNode(const HierarchyTreeNode* selectedNode)
{
    if (selectedNode == NULL)
    {
        // Nothing is selected - this is OK too.
        HandleNothingSelected();
        return;
    }

    this->activeNode = selectedNode;
    this->activeUIControlState = UIControlStateHelper::GetDefaultControlState();
  
    // Properties Grid View is ready to create the appropriate widgets.
    emit PropertiesGridUpdated();
}

void PropertiesGridController::UpdatePropertiesForUIControlNodeList(const HierarchyTreeController::SELECTEDCONTROLNODES
                                                                    &selectedNodes)
{
    if (selectedNodes.empty())
    {
        // Nothing is selected - this is OK too.
        // HandleNothingSelected(); TODO!!! IS THIS CORRECT???
        return;
    }

    this->activeNodes = selectedNodes;
    emit PropertiesGridUpdated();
}

void PropertiesGridController::HandleNothingSelected()
{
    CleanupSelection();
    emit PropertiesGridUpdated();
}

void PropertiesGridController::CleanupSelection()
{
    this->activeNodes.clear();
    this->activeNode = NULL;
}

void PropertiesGridController::OnSelectedStateChanged(UIControl::eControlState newState)
{
    SetActiveUIControlState(newState);
}
