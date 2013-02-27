#include "SceneGraphCommands.h"

#include "DAVAEngine.h"
#include "../Qt/Scene/SceneDataManager.h"
#include "../Qt/Scene/SceneData.h"
#include "../EditorScene.h"
#include "Scene3D/Components/DebugRenderComponent.h"
#include "CommandsManager.h"

using namespace DAVA;

CommandRemoveRootNodes::CommandRemoveRootNodes()
    :   Command(Command::COMMAND_UNDO_REDO)
    ,   activeScene(NULL)
{
	commandName = "Remove Root Nodes";
}


void CommandRemoveRootNodes::Execute()
{
    activeScene = SceneDataManager::Instance()->SceneGetActive();
    SceneNode *selectedNode = activeScene->GetSelectedNode();
    EditorScene *scene = activeScene->GetScene();
    if(selectedNode && scene && (selectedNode->GetParent() == scene))
    {
        //TODO: save scene state here
        
        
        String referenceToOwner;
        
        KeyedArchive *customProperties = selectedNode->GetCustomProperties();
        if(customProperties && customProperties->IsKeyExists("editor.referenceToOwner"))
        {
            referenceToOwner = customProperties->GetString("editor.referenceToOwner");
        }
        
        
        Vector<SceneNode *>nodesForDeletion;
        nodesForDeletion.reserve(scene->GetChildrenCount());
        
        for(int32 i = 0; i < scene->GetChildrenCount(); ++i)
        {
            SceneNode *node = scene->GetChild(i);
            
            customProperties = node->GetCustomProperties();
            if(customProperties && customProperties->IsKeyExists("editor.referenceToOwner"))
            {
                if(customProperties->GetString("editor.referenceToOwner") == referenceToOwner)
                {
                    nodesForDeletion.push_back(SafeRetain(node));
                }
            }
        }
        
        scene->SetSelection(NULL);
        activeScene->SelectNode(NULL);
        
        for(int32 i = 0; i < (int32)nodesForDeletion.size(); ++i)
        {
            SceneNode *node = nodesForDeletion[i];
            
            scene->ReleaseUserData(node);
            scene->RemoveNode(node);
            
            SafeRelease(node);
        }
        nodesForDeletion.clear();
        
//        SceneValidator::Instance()->EnumerateSceneTextures();
        
        SceneData *sceneData = SceneDataManager::Instance()->SceneGetActive();
        sceneData->RebuildSceneGraph();
    }
    else
    {
        SetState(STATE_INVALID);
    }
}

void CommandRemoveRootNodes::Cancel()
{
    //TODO: restore saved state if active scene is same as saved
}


CommandRefreshSceneGraph::CommandRefreshSceneGraph()
    :   Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
}


void CommandRefreshSceneGraph::Execute()
{
    SceneData * activeScene = SceneDataManager::Instance()->SceneGetActive();
    activeScene->RebuildSceneGraph();
}


CommandLockAtObject::CommandLockAtObject()
:   Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
}


void CommandLockAtObject::Execute()
{
    SceneData * activeScene = SceneDataManager::Instance()->SceneGetActive();
    SceneNode *node = activeScene->GetSelectedNode();
    if(node)
    {
        activeScene->LockAtSelectedNode();
    }
    else
    {
        SetState(STATE_INVALID);
    }
}



CommandRemoveSceneNode::CommandRemoveSceneNode()
:   Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
}

void CommandRemoveSceneNode::Execute()
{
	SceneData* activeScene = SceneDataManager::Instance()->SceneGetActive();
	SceneNode *node = activeScene->GetSelectedNode();

	CommandInternalRemoveSceneNode* command = new CommandInternalRemoveSceneNode(node);
	CommandsManager::Instance()->Execute(command);
	SafeRelease(command);
}


CommandInternalRemoveSceneNode::CommandInternalRemoveSceneNode(SceneNode* node)
:	Command(Command::COMMAND_UNDO_REDO)
{
	commandName = "Remove Object";

	this->node = SafeRetain(node);
	if (node)
	{
		nodeParent = node->GetParent();

		if (nodeParent)
		{
			insertBeforeNode = NULL;

			int32 i = GetNodeIndex(node);
			if (i < nodeParent->GetChildrenCount() - 1)
				insertBeforeNode = nodeParent->GetChild(i + 1);
		}
	}
}

CommandInternalRemoveSceneNode::~CommandInternalRemoveSceneNode()
{
	SafeRelease(node);
}

void CommandInternalRemoveSceneNode::Execute()
{
	if (!node || !nodeParent)
	{
		SetState(STATE_INVALID);
		return;
	}

	nodeParent->RemoveNode(node);

	SceneData *activeScene = SceneDataManager::Instance()->SceneGetActive();
	activeScene->RebuildSceneGraph();
}

void CommandInternalRemoveSceneNode::Cancel()
{
	if (!node || !nodeParent)
		return;

	if (insertBeforeNode)
	{
		int32 i = GetNodeIndex(insertBeforeNode);
		if (i < nodeParent->GetChildrenCount())
			nodeParent->InsertBeforeNode(node, insertBeforeNode);
		else
			nodeParent->AddNode(node);
	}
	else
		nodeParent->AddNode(node);

	SceneData *activeScene = SceneDataManager::Instance()->SceneGetActive();
	activeScene->RebuildSceneGraph();

	activeScene->SelectNode(node);
}

int32 CommandInternalRemoveSceneNode::GetNodeIndex(SceneNode* node)
{
	if (!node || !nodeParent)
		return -1;

	int32 i = 0;
	for (; i < nodeParent->GetChildrenCount(); ++i)
	{
		if (nodeParent->GetChild(i) == node)
			break;
	}

	return i;
}


CommandDebugFlags::CommandDebugFlags()
    :   Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
}


void CommandDebugFlags::Execute()
{
    SceneData * activeScene = SceneDataManager::Instance()->SceneGetActive();
    SceneNode *node = activeScene->GetSelectedNode();
    if(node)
    {
        if (node->GetDebugFlags() & DebugRenderComponent::DEBUG_DRAW_ALL)
        {
            node->SetDebugFlags(0, true);
        }
        else
        {
            node->SetDebugFlags(DebugRenderComponent::DEBUG_DRAW_ALL, true);
        }
    }
    else
    {
        SetState(STATE_INVALID);
    }
}


