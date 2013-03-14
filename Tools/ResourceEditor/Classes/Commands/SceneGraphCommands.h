#ifndef __SCENE_GRAPH_COMMANDS_H__
#define __SCENE_GRAPH_COMMANDS_H__

#include "Command.h"

class SceneData;
class DAVA::Entity;
class CommandRemoveRootNodes: public Command
{
public:	
	CommandRemoveRootNodes();

protected:
	virtual void Execute();
};


class CommandRefreshSceneGraph: public Command
{
public:	
	CommandRefreshSceneGraph();
    
protected:	
    
    virtual void Execute();
};

class CommandLockAtObject: public Command
{
public:
	CommandLockAtObject();
    
protected:
    
    virtual void Execute();
};


class CommandRemoveSceneNode: public Command
{
public:
	CommandRemoveSceneNode();

protected:
    virtual void Execute();
};

class CommandInternalRemoveSceneNode: public Command
{
public:
	CommandInternalRemoveSceneNode(DAVA::Entity* node, bool removeSimilar = false);
	virtual ~CommandInternalRemoveSceneNode();
	
protected:
	struct RemoveNodeRec
	{
		DAVA::Entity* node;
		DAVA::Entity* insertBeforeNode;
		DAVA::Entity* nodeParent;

		RemoveNodeRec()
		:	node(NULL)
		,	insertBeforeNode(NULL)
		,	nodeParent(NULL)
		{}
	};

	DAVA::Vector<RemoveNodeRec> nodesForDeletion;
	DAVA::Entity* selectedNode;

    virtual void Execute();
    virtual void Cancel();

	DAVA::int32 GetNodeIndex(const RemoveNodeRec& nodeRec);
};


class CommandDebugFlags: public Command
{
    
public:	
	CommandDebugFlags();
    
protected:	
    
    virtual void Execute();
};




#endif // #ifndef __SCENE_GRAPH_COMMANDS_H__