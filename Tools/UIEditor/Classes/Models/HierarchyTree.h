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
	bool Save(const QString& projectPath);
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
	
	bool IsProjectCreated() const {return projectCreated;};
	
    // Update the localization for all controls.
    void UpdateLocalization();

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
