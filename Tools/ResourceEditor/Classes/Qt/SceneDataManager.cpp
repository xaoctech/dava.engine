#include "SceneDataManager.h"

#include "SceneData.h"
#include "SceneGraphModel.h"

#include <QTreeView>

using namespace DAVA;

SceneDataManager::SceneDataManager()
    :   currentScene(NULL)
    ,   sceneGraphView(NULL)
{
}

SceneDataManager::~SceneDataManager()
{
    List<SceneData *>::iterator endIt = scenes.end();
    for(List<SceneData *>::iterator it = scenes.begin(); it != endIt; ++it)
    {
        SafeRelease(*it);
    }
    scenes.clear();
}

void SceneDataManager::ActivateScene(DAVA::Scene *scene)
{
    currentScene = FindDataForScene(scene);
    DVASSERT(currentScene && "There is no current scene. Something wrong.");
    
    DVASSERT(sceneGraphView && "QTreeView not initialized");
    currentScene->RebuildSceneGraph();
    sceneGraphView->setModel(currentScene->GetSceneGraph());
}

SceneData * SceneDataManager::FindDataForScene(DAVA::Scene *scene)
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

void SceneDataManager::RegisterNewScene(DAVA::Scene *scene)
{
    SceneData *data = new SceneData();
    data->SetScene(scene);

    scenes.push_back(data);
}

void SceneDataManager::ReleaseScene(DAVA::Scene *scene)
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
                
            SafeRelease(sceneData);

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

