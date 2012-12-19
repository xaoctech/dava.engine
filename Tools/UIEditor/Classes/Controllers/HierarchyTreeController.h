//
//  HierarchyTreeController.h
//  UIEditor
//
//  Created by Yuri Coder on 10/15/12.
//
//

#ifndef __UIEditor__HierarchyTreeController__
#define __UIEditor__HierarchyTreeController__

#include "DAVAEngine.h"

#include "HierarchyTree.h"

#include <QObject>
#include <QString>
#include <QPoint>
#include "HierarchyTreeScreenNode.h"
#include "HierarchyTreeControlNode.h"
#include "HierarchyTreePlatformNode.h"
#include <set>

using namespace DAVA;

// Hierarchy Tree Controller for handling UI Editor Project Hierarchy Tree.
class HierarchyTreeController: public QObject, public Singleton<HierarchyTreeController>
{
	Q_OBJECT
	
public:
	typedef std::set<HierarchyTreeControlNode*> SELECTEDCONTROLNODES;
	
	explicit HierarchyTreeController(QObject* parent = NULL);
    virtual ~HierarchyTreeController();
    
	bool NewProject(const QString& projectPath);
	bool Load(const QString& projectPath);
	bool Save(const QString& projectPath);

	void CloseProject();
	void AddPlatform(const QString& name, const Vector2& size);
	void AddScreen(const QString& name, HierarchyTreeNode::HIERARCHYTREENODEID platform);	
	void CreateNewControl(const QString& type, const QPoint& position);
	void DeleteNodes(const HierarchyTreeNode::HIERARCHYTREENODESLIST& nodes);
    
    const HierarchyTree& GetTree() const {return hierarchyTree;};
    
	void UpdateSelection(const HierarchyTreePlatformNode* activePlatform,
						 const HierarchyTreeScreenNode* activeScreen);

	void UpdateSelection(const HierarchyTreeNode* activeItem);
	
	void ChangeItemSelection(HierarchyTreeControlNode* control);
	void SelectControl(HierarchyTreeControlNode* control);
	void UnselectControl(HierarchyTreeControlNode* control);
	bool IsControlSelected(HierarchyTreeControlNode* control) const;
	void ResetSelectedControl();
	
	HierarchyTreePlatformNode* GetActivePlatform() const;
    HierarchyTreeScreenNode* GetActiveScreen() const;
	
    void EmitHierarchyTreeUpdated();

    const SELECTEDCONTROLNODES& GetActiveControlNodes() const;
	bool IsNodeActive(const HierarchyTreeControlNode* activeControl) const;

    // Look through all controls and update their localized texts.
    void UpdateLocalization(bool takePathFromLocalizationSystem);

private:
	void DeleteNodesInternal(const HierarchyTreeNode::HIERARCHYTREENODESLIST& nodes);
	String GetNewControlName(const String& baseName);
	
signals:
	void ProjectCreated();
	void ProjectClosed();
	void ProjectLoaded();
	void ProjectSaved();
	void PlatformCreated();
//	void PlatformDeleted();
//	void ScreenCreated();
//	void Scree
	
	void HierarchyTreeUpdated();
	void SelectedPlatformChanged(const HierarchyTreePlatformNode*);
	void SelectedScreenChanged(const HierarchyTreeScreenNode*);
	
	void AddSelectedControl(const HierarchyTreeControlNode*);
	void RemoveSelectedControl(const HierarchyTreeControlNode*);
	void SelectedControlNodesChanged(const HierarchyTreeController::SELECTEDCONTROLNODES &);
	
	void SelectedTreeItemChanged(const HierarchyTreeNode*);
		
protected:
	void Clear();
	
    // Hierarchy Tree.
    HierarchyTree hierarchyTree;
    
	HierarchyTreePlatformNode* activePlatform;
    HierarchyTreeScreenNode* activeScreen;
//    HierarchyTreeControlNode* activeControl;
    
	SELECTEDCONTROLNODES activeControlNodes;
};

#endif /* defined(__UIEditor__HierarchyTreeController__) */
