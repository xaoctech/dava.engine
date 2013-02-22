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

protected:
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

private:
	QString name;
	HierarchyTreeNode::HIERARCHYTREENODEID platformId;
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

class DeleteSelectedNodeCommand: public BaseCommand
{
public:
	DeleteSelectedNodeCommand(const HierarchyTreeNode::HIERARCHYTREENODESLIST& nodes);
	
	virtual void Execute();
	void Rollback();
	virtual bool IsUndoRedoSupported() {return true;};
	
private:
	HierarchyTreeNode::HIERARCHYTREENODESLIST nodes;
	
	// Prepare the information needed for Redo.
	void PrepareRedoInformation();
	
	// Information needed during Undo/Redo.
	HierarchyTreeNode::HIERARCHYTREENODESLIST redoNodes;
};

class ChangeNodeHeirarchy: public BaseCommand
{
public:
	ChangeNodeHeirarchy(HierarchyTreeNode::HIERARCHYTREENODEID targetNodeID, HierarchyTreeNode::HIERARCHYTREENODESIDLIST items);

	virtual void Execute();
	virtual void Rollback();
	virtual bool IsUndoRedoSupported() {return true;};
	
protected:
	// Store the previous parents to apply Rollback.
	void StorePreviousParents();

private:
	HierarchyTreeNode::HIERARCHYTREENODEID targetNodeID;
	HierarchyTreeNode::HIERARCHYTREENODESIDLIST items;

	// Previous parents for the "items" list.
	typedef Map<HierarchyTreeNode::HIERARCHYTREENODEID, HierarchyTreeNode::HIERARCHYTREENODEID> PARENTNODESMAP;
	typedef PARENTNODESMAP::iterator PARENTNODESMAPITER;

	PARENTNODESMAP previousParents;
};

#endif /* defined(__UIEditor__ItemsCommand__) */
