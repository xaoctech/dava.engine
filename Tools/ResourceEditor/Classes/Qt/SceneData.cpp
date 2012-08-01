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
    sceneGraphModel->SetScene(scene);
}


Scene * SceneData::GetScene()
{
    return scene;
}
