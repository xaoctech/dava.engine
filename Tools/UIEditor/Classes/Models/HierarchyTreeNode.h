//
//  HierarchyTreeNode.h
//  UIEditor
//
//  Created by Yuri Coder on 10/15/12.
//
//

#ifndef __UIEditor__HierarchyTreeNode__
#define __UIEditor__HierarchyTreeNode__

#include <set>
#include "DAVAEngine.h"
#include <QString>
#include "HierarchyTreeNodeExtraData.h"

using namespace DAVA;

// Base class for all Hierarchy Tree Nodes.
class HierarchyTreeNode
{
public:
    // Type definitions for the Tree Node.
	typedef std::list<HierarchyTreeNode*> HIERARCHYTREENODESLIST;
    typedef HIERARCHYTREENODESLIST::iterator HIERARCHYTREENODESITER;
    typedef HIERARCHYTREENODESLIST::const_iterator HIERARCHYTREENODESCONSTITER;
    
	typedef int HIERARCHYTREENODEID;
	typedef std::list<HIERARCHYTREENODEID> HIERARCHYTREENODESIDLIST;
  	static const HIERARCHYTREENODEID HIERARCHYTREENODEID_EMPTY = -1;

    HierarchyTreeNode(const QString& name);
	HierarchyTreeNode(const HierarchyTreeNode* node);
    virtual ~HierarchyTreeNode();
    
    // Add the node to the list.
    void AddTreeNode(HierarchyTreeNode* treeNode);
    void AddTreeNode(HierarchyTreeNode* treeNode, HierarchyTreeNode* nodeToAddAfter);

    // Remove the node from the list, return TRUE if succeeded.
    bool RemoveTreeNode(HierarchyTreeNode* treeNode, bool needDelete, bool needRemoveFromScene);
    
    // Access to the nodes list.
    const HIERARCHYTREENODESLIST& GetChildNodes() const;
    
	virtual void SetName(const QString& name) {this->name = name;};
    const QString& GetName() const {return name;};
	
	HIERARCHYTREENODEID GetId() const {return id;};
	void UpdateId(HIERARCHYTREENODEID newID) { this->id = newID; };

    // Access to the node extra data.
    HierarchyTreeNodeExtraData& GetExtraData() {return extraData;};
	
	virtual void SetParent(HierarchyTreeNode* /*node*/, HierarchyTreeNode* /*insertAfter*/){};
	virtual HierarchyTreeNode* GetParent() {return NULL;};

	bool IsHasChild(const HierarchyTreeNode* node) const;

	// Remove the tree node from scene, but keep it in memory.
	virtual void RemoveTreeNodeFromScene() {};
	
	// Return it back to the scene.
	virtual void ReturnTreeNodeToScene() {};

	// Prepare the Undo/Redo information.
	void PrepareRemoveFromSceneInformation();

	virtual bool IsMarked() const;
	virtual bool IsNeedSave() const;
	bool HasUnsavedChilder() const;
	void SetMarked(bool marked);
	void SetChildrenMarked(bool marked, bool recursive = false);

	// Modifiers for the unsaved changes counter.
	void IncrementUnsavedChanges();
	void DecrementUnsavedChanges();
	void ResetUnsavedChanges();

protected:
	HIERARCHYTREENODEID id;
	
	QString name;
	
    // Cleanup the list of tree nodes.
    void Cleanup();
    
    // List of child nodes.
    HIERARCHYTREENODESLIST childNodes;
    
    // Tree node extra data.
    HierarchyTreeNodeExtraData extraData;
	
	static HIERARCHYTREENODEID nextId;
	
	// Undo/Redo information.
	HierarchyTreeNode* redoParentNode;
	HierarchyTreeNode* redoPreviousNode;

	bool marked;
	int32 unsavedChangesCounter;
};


#endif /* defined(__UIEditor__HierarchyTreeNode__) */
