#ifndef __SCENE_GRAPH_MODEL_H__
#define __SCENE_GRAPH_MODEL_H__

#include "GraphModel.h"


class SceneGraphModel: public GraphModel
{
public:
    SceneGraphModel(QObject *parent = 0);
    virtual ~SceneGraphModel();

protected:

    virtual void SetupModelData();

};

#endif // __SCENE_GRAPH_MODEL_H__
