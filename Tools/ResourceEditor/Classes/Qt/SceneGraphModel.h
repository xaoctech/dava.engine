#ifndef __SCENE_GRAPH_MODEL_H__
#define __SCENE_GRAPH_MODEL_H__

#include "DAVAEngine.h"
#include "GraphModel.h"

class SceneGraphItem;
class SceneGraphModel: public GraphModel
{
public:
    SceneGraphModel(QObject *parent = 0);
    virtual ~SceneGraphModel();

    void SetScene(DAVA::Scene * newScene);
    
protected:
    
    void AddNodeToTree(GraphItem *parent, DAVA::SceneNode *node);
    
protected:

    DAVA::Scene *scene;

};

#endif // __SCENE_GRAPH_MODEL_H__
