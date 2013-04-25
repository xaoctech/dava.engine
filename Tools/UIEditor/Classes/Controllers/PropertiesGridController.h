//
//  PropertiesGridController.h
//  UIEditor
//
//  Created by Yuri Coder on 10/18/12.
//
//

#ifndef __UIEditor__PropertiesGridController__
#define __UIEditor__PropertiesGridController__

#include <QObject>

#include "HierarchyTreeController.h"
#include "HierarchyTreeNode.h"
#include "HierarchyTreeControlNode.h"
#include "BaseMetadata.h"

#include "UI/UIControl.h"

namespace DAVA {

// A controller for Properties Grid.
class PropertiesGridController : public QObject, public Singleton<PropertiesGridController>
{
    Q_OBJECT
public:
    PropertiesGridController(QObject* parent = 0);
    ~PropertiesGridController();
    
    // Get the active tree node or nodes list.
    const HierarchyTreeNode* GetActiveTreeNode() const;
    const HierarchyTreeController::SELECTEDCONTROLNODES GetActiveTreeNodesList() const;

    // Access to the active UI Control State.
    Vector<UIControl::eControlState> GetActiveUIControlStates() const;
    void SetActiveUIControlStates(const Vector<UIControl::eControlState>& newStates);

signals:
    // Generated when Properties Grid is updated and needs to be re-built.
    void PropertiesGridUpdated();
    
    // Generated when UI Control State is changed.
    void SelectedUIControlStatesChanged(const Vector<UIControl::eControlState>& newState);

public slots:
    // Emitted by Hierarchy Tree Controller when selected control is changed.
    void OnSelectedTreeItemChanged(const HierarchyTreeNode* selectedNode);
    
    // Emitted by Hierarchy Tree Controller when Control Nodes selection is changed
    // (can contain one or more Control Nodes).
    void OnSelectedControlNodesChanged(const HierarchyTreeController::SELECTEDCONTROLNODES &selectedNodes);
    
    // Selected State is changed.
	void OnSelectedStateChanged(UIControl::eControlState newState);
	void OnSelectedStatesChanged(const Vector<UIControl::eControlState>& newStates);

protected:
    // Connect to the signals.
    void ConnectToSignals();

    // Update the Properties Grid for the single node and for multiple nodes.
    void UpdatePropertiesForSingleNode(const HierarchyTreeNode* selectedNode);
    void UpdatePropertiesForUIControlNodeList(const HierarchyTreeController::SELECTEDCONTROLNODES &selectedNodes);
    
    // Handle situation when nothing in tree is selectted.
    void HandleNothingSelected();

    // Cleanup the selection remembered.
    void CleanupSelection();
    
private:
    // Tree Node currently selected.
    const HierarchyTreeNode* activeNode; // TODO! REMOVE THIS!!!

    // List of Tree Nodes currently selected.
    HierarchyTreeController::SELECTEDCONTROLNODES activeNodes;
    
    // Active UI Control State.
	Vector<UIControl::eControlState> activeUIControlStates;
    
    //Current localization files directory
    QString localizationDirectoryPath;
};
    
}

#endif /* defined(__UIEditor__PropertiesGridController__) */
