//
//  HierarchyTreeAggregatorNode.h
//  UIEditor
//
//  Created by adebt on 3/11/13.
//
//

#ifndef __UIEditor__HierarchyTreeAggregatorNode__
#define __UIEditor__HierarchyTreeAggregatorNode__

#include "HierarchyTreeScreenNode.h"
#include "HierarchyTreeControlNode.h"

namespace DAVA
{
	
class HierarchyTreeAggregatorNode: public HierarchyTreeScreenNode
{
public:
	typedef Set<HierarchyTreeControlNode*> CHILDS;

	HierarchyTreeAggregatorNode(HierarchyTreePlatformNode* parent, const QString& name, const Rect& rect);
	~HierarchyTreeAggregatorNode();

	void AddChild(HierarchyTreeControlNode* node);
	void RemoveChild(HierarchyTreeControlNode* node);
	const CHILDS& GetChilds() const;
	
	HierarchyTreeControlNode* CreateChild(HierarchyTreeNode* parentNode, const QString& name);
	void UpdateChilds();
	
	void SetRect(const Rect& rect);
	virtual Rect GetRect() const;
	virtual void SetParent(HierarchyTreeNode* node, HierarchyTreeNode* insertAfter);
	virtual void RemoveSelection();
	
	bool Load(YamlNode* node, const QString& path);
	bool Save(YamlNode* node, const QString& path);
	
	virtual void SetName(const QString& name);
	
private:
	void CopyAggregatorControls();
	void UpdateHierarchyTree();
	void ReplaceAggregator(HierarchyTreeControlNode* node);
	
private:
	Rect rect;
	
	CHILDS childs;
	
	String path;
};
	
}

#endif /* defined(__UIEditor__HierarchyTreeAggregatorNode__) */
