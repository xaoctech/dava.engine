#include "SceneData.h"
#include "SceneGraphModel.h"

using namespace DAVA;

SceneData::SceneData()
    :   scene(NULL)
    ,   selectedNode(NULL)
    ,   sceneGraphModel(NULL)
{
}

SceneData::~SceneData()
{
    SafeRelease(scene);
    SafeDelete(sceneGraphModel);
}


SceneGraphModel * SceneData::GetSceneGraph()
{
    return sceneGraphModel;
}


void SceneData::SetScene(DAVA::Scene *newScene)
{
    SafeDelete(sceneGraphModel);
    SafeRelease(scene);
    
    scene = SafeRetain(newScene);
    sceneGraphModel = new SceneGraphModel(NULL);
    RebuildSceneGraph();
}

void SceneData::RebuildSceneGraph()
{
    Scene *sceneForRebuild = SafeRetain(scene);
    sceneGraphModel->SetScene(sceneForRebuild);
    SafeRelease(sceneForRebuild);
}


Scene * SceneData::GetScene()
{
    return scene;
}
