//
//  ItemsCommand.h
//  UIEditor
//
//  Created by adebt on 10/29/12.
//
//

#ifndef __UIEditor__ItemsCommand__
#define __UIEditor__ItemsCommand__

#include "BaseCommand.h"
#include "HierarchyTreeNode.h"
#include <QString>
#include <QPoint>

using namespace DAVA;

// Base command for Undoable commands which creates differnt Nodes.
class UndoableHierarchyTreeNodeCommand : public BaseCommand
{
public:
	UndoableHierarchyTreeNodeCommand();

	// Prepare the information needed to remove node from the scene.
	void PrepareRemoveFromSceneInformation();

	// Set the Redo Node.
	void SetRedoNode(HierarchyTreeNode* redoNode);

	// Return the Redo Node to scene.
	void ReturnRedoNodeToScene();

	void DetectType(HierarchyTreeNode* node);
protected:
	// the type of objects on which command is being performed
	enum eObjectType
	{
		TYPE_UNDEFINED = -1,
		TYPE_PLATFORM = 0,
		TYPE_AGGREGATOR,
		TYPE_SCREEN,
		TYPE_CONTROLS,
		TYPES_COUNT
	};

	eObjectType type;

	// The Redo Node remembered.
	HierarchyTreeNode* redoNode;
};

class CreatePlatformCommand: public UndoableHierarchyTreeNodeCommand
{
public:
	CreatePlatformCommand(const QString& name, const Vector2& size);
    
	virtual void Execute();
	virtual void Rollback();

	virtual bool IsUndoRedoSupported() {return true;};

	virtual void IncrementUnsavedChanges();
	virtual void DecrementUnsavedChanges();

private:
	QString name;
	Vector2 size;
};

class CreateScreenCommand: public UndoableHierarchyTreeNodeCommand
{
public:
	CreateScreenCommand(const QString& name, HierarchyTreeNode::HIERARCHYTREENODEID platformId);
	
	virtual void Execute();
	virtual void Rollback();

	virtual bool IsUndoRedoSupported() {return true;};

	virtual void IncrementUnsavedChanges();
	virtual void DecrementUnsavedChanges();
private:
	QString name;
	HierarchyTreeNode::HIERARCHYTREENODEID platformId;
};

class CreateAggregatorCommand: public UndoableHierarchyTreeNodeCommand
{
public:
	CreateAggregatorCommand(const QString& name, HierarchyTreeNode::HIERARCHYTREENODEID platformId, const Rect& rect);
	
	virtual void Execute();
	virtual void Rollback();
	
	virtual bool IsUndoRedoSupported() {return true;};

	virtual void IncrementUnsavedChanges();
	virtual void DecrementUnsavedChanges();
private:
	QString name;
	HierarchyTreeNode::HIERARCHYTREENODEID platformId;
	Rect rect;
};

class CreateControlCommand: public BaseCommand
{
public:
	CreateControlCommand(const QString& type, const QPoint& pos);
	
	virtual void Execute();
	void Rollback();
	virtual bool IsUndoRedoSupported() {return true;};

private:
	QString type;
	QPoint pos;

	HierarchyTreeNode::HIERARCHYTREENODEID createdControlID;
	
	// Prepare the information needed for Redo.
	void PrepareRedoInformation();
	
	// Information needed during Undo/Redo.
	HierarchyTreeNode* redoNode;
};

class DeleteSelectedNodeCommand: public UndoableHierarchyTreeNodeCommand
{
public:
	DeleteSelectedNodeCommand(const HierarchyTreeNode::HIERARCHYTREENODESLIST& nodes, bool needDeleteFiles = false);
	
	virtual void Execute();
	void Rollback();
	virtual bool IsUndoRedoSupported() {return true;};

	virtual void IncrementUnsavedChanges();
	virtual void DecrementUnsavedChanges();

private:
	HierarchyTreeNode::HIERARCHYTREENODEID parentId;

	HierarchyTreeNode::HIERARCHYTREENODESLIST nodes;
	
	// Prepare the information needed for Redo.
	void PrepareRedoInformation();

	// Need to store parents of UIAggregatorControl
	// removing from scene with corresponding HierarchyTreeAggregatorNode
	void StoreParentsOfRemovingAggregatorControls(const HierarchyTreeNode::HIERARCHYTREENODESLIST& nodes);

	// Information needed during Undo/Redo.
	HierarchyTreeNode::HIERARCHYTREENODESLIST redoNodes;
	Set<HierarchyTreeNode::HIERARCHYTREENODEID> parentsOfRemovingAggregatorControls;

	bool deleteFiles;
};

class ChangeNodeHeirarchy: public UndoableHierarchyTreeNodeCommand
{
public:
	ChangeNodeHeirarchy(HierarchyTreeNode::HIERARCHYTREENODEID targetNodeID, HierarchyTreeNode::HIERARCHYTREENODEID afterNodeID, HierarchyTreeNode::HIERARCHYTREENODESIDLIST items);

	virtual void Execute();
	virtual void Rollback();
	virtual bool IsUndoRedoSupported() {return true;};

	virtual void IncrementUnsavedChanges();
	virtual void DecrementUnsavedChanges();

protected:
	// Store the previous parents to apply Rollback.
	void StorePreviousParents();

private:
	HierarchyTreeNode::HIERARCHYTREENODEID targetNodeID;
	HierarchyTreeNode::HIERARCHYTREENODESIDLIST items;

	// Previous marked flags for Screens
	Map<HierarchyTreeNode::HIERARCHYTREENODEID, bool> storedMarks;

	// Previous parents for the "items" list.
	struct PreviousState
	{
		HierarchyTreeNode::HIERARCHYTREENODEID parent;
		HierarchyTreeNode::HIERARCHYTREENODEID addedAfter;
		PreviousState(HierarchyTreeNode::HIERARCHYTREENODEID parent, HierarchyTreeNode::HIERARCHYTREENODEID addedAfter)
		{
			this->parent = parent;
			this->addedAfter = addedAfter;
		}
	};
	typedef Map<HierarchyTreeNode::HIERARCHYTREENODEID, PreviousState> PARENTNODESMAP;
	typedef PARENTNODESMAP::iterator PARENTNODESMAPITER;

	PARENTNODESMAP previousParents;
	HierarchyTreeNode::HIERARCHYTREENODEID afterNodeID;
};

#endif /* defined(__UIEditor__ItemsCommand__) */
