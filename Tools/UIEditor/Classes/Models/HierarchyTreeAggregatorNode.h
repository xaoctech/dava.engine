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
	HierarchyTreeAggregatorNode(HierarchyTreePlatformNode* parent,
								const HierarchyTreeAggregatorNode* base);
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
	
	bool Load(const Rect& rect, const QString& path);
	bool Save(YamlNode* node, const QString& path, bool saveAll);
	
	virtual void SetName(const QString& name);
	
	const String& GetPath();

	void UpdateHierarchyTree();
private:
	void CopyAggregatorControls();
	void ReplaceAggregator(HierarchyTreeControlNode* node);
	
private:
	Rect rect;
	
	CHILDS childs;
	
	String path;
};
	
}

#endif /* defined(__UIEditor__HierarchyTreeAggregatorNode__) */
