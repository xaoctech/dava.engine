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

class CreatePlatformCommand: public BaseCommand
{
public:
	CreatePlatformCommand(const QString& name, const Vector2& size);
    
	virtual void Execute();
	
private:
	QString name;
	Vector2 size;
};

class CreateScreenCommand: public BaseCommand
{
public:
	CreateScreenCommand(const QString& name, HierarchyTreeNode::HIERARCHYTREENODEID platformId);
	
	virtual void Execute();
	
private:
	QString name;
	HierarchyTreeNode::HIERARCHYTREENODEID platformId;
};

class CreateControlCommand: public BaseCommand
{
public:
	CreateControlCommand(const QString& type, const QPoint& pos);
	
	virtual void Execute();
	
private:
	QString type;
	QPoint pos;
};

class DeleteSelectedNodeCommand: public BaseCommand
{
public:
	DeleteSelectedNodeCommand(const HierarchyTreeNode::HIERARCHYTREENODESLIST& nodes);
	
	virtual void Execute();
	
private:
	HierarchyTreeNode::HIERARCHYTREENODESLIST nodes;
};

class ChangeNodeHeirarchy: public BaseCommand
{
public:
	ChangeNodeHeirarchy(HierarchyTreeNode* targetNode, HierarchyTreeNode::HIERARCHYTREENODESIDLIST items);

	virtual void Execute();
	
private:
	HierarchyTreeNode* targetNode;
	HierarchyTreeNode::HIERARCHYTREENODESIDLIST items;
};

#endif /* defined(__UIEditor__ItemsCommand__) */
