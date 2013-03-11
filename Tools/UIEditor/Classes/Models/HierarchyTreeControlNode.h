//
//  HierarchyTreeControlNode.h
//  UIEditor
//
//  Created by Yuri Coder on 10/15/12.
//
//

#ifndef __UIEditor__HierarchyTreeControlNode__
#define __UIEditor__HierarchyTreeControlNode__

#include "DAVAEngine.h"
#include "HierarchyTreeNode.h"
#include "HierarchyTreeScreenNode.h"

using namespace DAVA;

// "Control" node for the Hierarchy Tree.
class HierarchyTreeControlNode: public HierarchyTreeNode
{
public:
	HierarchyTreeControlNode(HierarchyTreeNode* parent, UIControl* uiObject, const QString& name);
	HierarchyTreeControlNode(HierarchyTreeNode* parent, const HierarchyTreeControlNode* node);
	~HierarchyTreeControlNode();

	HierarchyTreeScreenNode* GetScreenNode() const;
	HierarchyTreeControlNode* GetControlNode() const;
	UIControl* GetUIObject() const {return uiObject;};
	
	virtual void SetParent(HierarchyTreeNode* node, HierarchyTreeNode* insertAfter);
	virtual HierarchyTreeNode* GetParent() {return parent;};
	
	Vector2 GetParentDelta(bool skipControl = false) const;

	// Remove/return Tree Node from the scene.
	virtual void RemoveTreeNodeFromScene();
	virtual void ReturnTreeNodeToScene();
	
	Rect GetRect() const;

private:
	void AddControlToParent();
	
private:
	HierarchyTreeNode* parent;

	UIControl* uiObject;

	UIControl* parentUIObject;
	UIControl* childUIObjectAbove;
	bool needReleaseUIObjects;
};

#endif /* defined(__UIEditor__HierarchyTreeControlNode__) */
