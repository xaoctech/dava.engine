#include "SceneDataManager.h"

#include "SceneData.h"
#include "SceneGraphModel.h"

#include "../EditorScene.h"

#include <QTreeView>

using namespace DAVA;

SceneDataManager::SceneDataManager()
    :   currentScene(NULL)
    ,   sceneGraphView(NULL)
    ,   libraryView(NULL)
{
}

SceneDataManager::~SceneDataManager()
{
    List<SceneData *>::iterator endIt = scenes.end();
    for(List<SceneData *>::iterator it = scenes.begin(); it != endIt; ++it)
    {
        SafeDelete(*it);
    }
    scenes.clear();
}

void SceneDataManager::ActivateScene(EditorScene *scene)
{
    if(currentScene)
    {
        currentScene->Deactivate();
    }
    
    
    currentScene = FindDataForScene(scene);
    DVASSERT(currentScene && "There is no current scene. Something wrong.");
    
    DVASSERT(sceneGraphView && "QTreeView not initialized");
    currentScene->RebuildSceneGraph();
    currentScene->Activate(sceneGraphView, libraryView);
}

SceneData * SceneDataManager::FindDataForScene(EditorScene *scene)
{
    SceneData *foundData = NULL;

    List<SceneData *>::iterator endIt = scenes.end();
    for(List<SceneData *>::iterator it = scenes.begin(); it != endIt; ++it)
    {
        if((*it)->GetScene() == scene)
        {
            foundData = *it;
            break;
        }
    }
    
    return foundData;
}


SceneData * SceneDataManager::GetActiveScene()
{
    return currentScene;
}

SceneData *SceneDataManager::GetLevelScene()
{
    if(0 < scenes.size())
    {
        return scenes.front();
    }
    
    return NULL;
}

EditorScene * SceneDataManager::RegisterNewScene()
{
    SceneData *data = new SceneData();
    data->CreateScene(true);

    scenes.push_back(data);
    
    return data->GetScene();
}

void SceneDataManager::ReleaseScene(EditorScene *scene)
{
    List<SceneData *>::iterator endIt = scenes.end();
    for(List<SceneData *>::iterator it = scenes.begin(); it != endIt; ++it)
    {
        SceneData *sceneData = *it;
        if(sceneData->GetScene() == scene)
        {
            if(currentScene == sceneData)
            {
                DVASSERT((0 < scenes.size()) && "There is no main level scene.")
                currentScene = *scenes.begin(); // maybe we need to activate next or prev tab?
            }
                
            SafeDelete(sceneData);

            scenes.erase(it);
            break;
        }
    }
}

DAVA::int32 SceneDataManager::ScenesCount()
{
    return (int32)scenes.size();
}

void SceneDataManager::SetSceneGraphView(QTreeView *view)
{
    sceneGraphView = view;
}

void SceneDataManager::SetLibraryView(QTreeView *view)
{
    libraryView = view;
}

