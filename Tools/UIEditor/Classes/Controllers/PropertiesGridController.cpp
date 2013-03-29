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
    this->activeUIControlStates.push_back(UIControl::STATE_NORMAL);
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

Vector<UIControl::eControlState> PropertiesGridController::GetActiveUIControlStates() const
{
	return this->activeUIControlStates;
}

void PropertiesGridController::SetActiveUIControlStates(const Vector<UIControl::eControlState>& newStates)
{
	bool stateChanged = false;

	if (newStates.size() != activeUIControlStates.size())
	{
		stateChanged = true;
	}
	else
	{
		for (uint32 i = 0; i < activeUIControlStates.size(); ++i)
		{
			stateChanged |= (std::find(newStates.begin(), newStates.end(), activeUIControlStates[i]) ==
							 newStates.end());

			if (stateChanged)
			{
				break;
			}
		}
	}

	if (stateChanged)
	{
		this->activeUIControlStates = newStates;
		emit SelectedUIControlStatesChanged(newStates);
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

	this->activeUIControlStates.clear();
    this->activeUIControlStates.push_back(UIControlStateHelper::GetDefaultControlState());
  
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
	Vector<UIControl::eControlState> newStates;
	newStates.push_back(newState);

	SetActiveUIControlStates(newStates);
}

void PropertiesGridController::OnSelectedStatesChanged(const Vector<UIControl::eControlState>& newStates)
{
	SetActiveUIControlStates(newStates);
}