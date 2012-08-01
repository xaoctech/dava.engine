#ifndef __SCENE_DATA_H__
#define __SCENE_DATA_H__

#include "DAVAEngine.h"

class SceneGraphModel;
class SceneData: public DAVA::BaseObject
{
public:
    SceneData();
    virtual ~SceneData();

    SceneGraphModel *GetSceneGraph();
    
    void SetScene(DAVA::Scene *newScene);
    DAVA::Scene * GetScene();
    
    
protected:

    DAVA::Scene *scene;
    DAVA::SceneNode *selectedNode;
    
    SceneGraphModel *sceneGraphModel;
    //DATA
    //ENTITY
    //PROPERTY
    //LIBRARY
};

#endif // __SCENE_DATA_H__
