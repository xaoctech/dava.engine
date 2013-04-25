//
//  HierarchyTree.h
//  UIEditor
//
//  Created by Yuri Coder on 10/15/12.
//
//

#ifndef __UIEditor__HierarchyTree__
#define __UIEditor__HierarchyTree__

#include "HierarchyTreeRootNode.h"
#include "HierarchyTreePlatformNode.h"
#include "HierarchyTreeControlNode.h"
#include "HierarchyTreeAggregatorNode.h"

#include "BaseMetadata.h"

#include <QString>

using namespace DAVA;

// Hierarchy Tree.
class HierarchyTree
{
public:
	HierarchyTree();

    bool Load(const QString& projectPath);
	
	bool SaveOnlyChangedScreens(const QString& projectPath);
	bool SaveAll(const QString& projectPath);

	const QString& GetActiveProjectPath() const {return rootNode.GetProjectDir();};
	
	void CreateProject();
	void CloseProject();

	HierarchyTreePlatformNode* AddPlatform(const QString& name, const Vector2& size);
	HierarchyTreeScreenNode* AddScreen(const QString& name, HierarchyTreeNode::HIERARCHYTREENODEID platformId);
	HierarchyTreeAggregatorNode* AddAggregator(const QString& name, HierarchyTreeNode::HIERARCHYTREENODEID platformId, const Rect& rect);

	void DeleteNodes(const HierarchyTreeNode::HIERARCHYTREENODESLIST& nodes, bool deleteNodeFromMemory,
					 bool deleteNodeFromScene);
 	
	const HierarchyTreeRootNode* GetRootNode() const {return &rootNode;};
   
	HierarchyTreeNode* GetNode(HierarchyTreeNode::HIERARCHYTREENODEID id) const;
	HierarchyTreeNode* GetNode(const UIControl* control) const;
	
	const HierarchyTreeNode::HIERARCHYTREENODESLIST& GetPlatforms() const;

	// Whether the project is created?
	bool IsProjectCreated() const {return projectCreated;};

	// Returns the list of screens currently unsaved. Might be empty.
	List<HierarchyTreeScreenNode*> GetUnsavedScreens();

    // Update the localization for all controls.
    void UpdateLocalization();

	bool IsPlatformNamePresent(const QString& name) const ;

protected:
	// Do the save itself for only changed screens or for all screens.
	bool DoSave(const QString& projectPath, bool saveAll);

private:
	void Clear();
    
    // Update Extra Data from/to the control's data.
    void UpdateExtraData(BaseMetadata::eExtraDataUpdateStyle updateStyle);
    void UpdateExtraDataRecursive(HierarchyTreeControlNode* node, BaseMetadata::eExtraDataUpdateStyle updateStyle);

private:
	HierarchyTreeNode* FindNode(const HierarchyTreeNode* parent, HierarchyTreeNode::HIERARCHYTREENODEID id) const;
	HierarchyTreeNode* FindNode(const HierarchyTreeNode* parent, const UIControl* control) const;
    HierarchyTreeRootNode rootNode;
	
	bool projectCreated;
};

#endif /* defined(__UIEditor__HierarchyTree__) */
