#include "SceneGraphCommands.h"

#include "DAVAEngine.h"
#include "../Qt/SceneDataManager.h"
#include "../Qt/SceneData.h"
#include "../EditorScene.h"

using namespace DAVA;

CommandRemoveRootNodes::CommandRemoveRootNodes()
    :   Command(Command::COMMAND_UNDO_REDO)
    ,   activeScene(NULL)
{
}


void CommandRemoveRootNodes::Execute()
{
    activeScene = SceneDataManager::Instance()->GetActiveScene();
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
        
        SceneData *sceneData = SceneDataManager::Instance()->GetActiveScene();
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
    SceneData * activeScene = SceneDataManager::Instance()->GetActiveScene();
    activeScene->RebuildSceneGraph();
}


CommandLockAtObject::CommandLockAtObject()
:   Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
}


void CommandLockAtObject::Execute()
{
    SceneData * activeScene = SceneDataManager::Instance()->GetActiveScene();
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
    :   Command(Command::COMMAND_UNDO_REDO)
    ,   activeScene(NULL)
{
}


void CommandRemoveSceneNode::Execute()
{
    activeScene = SceneDataManager::Instance()->GetActiveScene();
    SceneNode *node = activeScene->GetSelectedNode();
    if(node)
    {
        //TODO: save scene state here
        SceneData *activeScene = SceneDataManager::Instance()->GetActiveScene();
        activeScene->RemoveSceneNode(node);
    }
    else
    {
        SetState(STATE_INVALID);
    }
}

void CommandRemoveSceneNode::Cancel()
{
    //TODO: restore saved state if active scene is same as saved
}


CommandDebugFlags::CommandDebugFlags()
    :   Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
}


void CommandDebugFlags::Execute()
{
    SceneData * activeScene = SceneDataManager::Instance()->GetActiveScene();
    SceneNode *node = activeScene->GetSelectedNode();
    if(node)
    {
        if (node->GetDebugFlags() & SceneNode::DEBUG_DRAW_ALL)
        {
            node->SetDebugFlags(0, true);
        }
        else
        {
            node->SetDebugFlags(SceneNode::DEBUG_DRAW_ALL, true);
        }
    }
    else
    {
        SetState(STATE_INVALID);
    }
}


CommandBakeMatrices::CommandBakeMatrices()
    :   Command(Command::COMMAND_UNDO_REDO)
    ,   activeScene(NULL)
{
}


void CommandBakeMatrices::Execute()
{
    activeScene = SceneDataManager::Instance()->GetActiveScene();

    SceneNode *node = activeScene->GetSelectedNode();
    if(node)
    {
        //Safe node state
        node->BakeTransforms();
    }
    else
    {
        SetState(STATE_INVALID);
    }
}

void CommandBakeMatrices::Cancel()
{
    //TODO: restore saved state if active scene is same as saved
}


CommandBuildQuadTree::CommandBuildQuadTree()
:   Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
}


void CommandBuildQuadTree::Execute()
{
    SceneData * activeScene = SceneDataManager::Instance()->GetActiveScene();
    SceneNode *node = activeScene->GetSelectedNode();
    if(node)
    {
//        Scene * scene = workingNode->GetScene();
//        QuadTree * quadTree = dynamic_cast<QuadTree*>(scene->GetBVHierarchy());
//        if (!quadTree)
//        {
//            quadTree = new QuadTree();
//            scene->SetBVHierarchy(quadTree);
//        }
//        quadTree->Build(scene);
    }
    else
    {
        SetState(STATE_INVALID);
    }
}
