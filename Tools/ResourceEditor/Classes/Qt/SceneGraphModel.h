#ifndef __SCENE_GRAPH_MODEL_H__
#define __SCENE_GRAPH_MODEL_H__

#include "DAVAEngine.h"
#include "GraphModel.h"

#include <QObject>

class EditorScene;
class SceneGraphItem;
class SceneGraphModel: public GraphModel
{
    Q_OBJECT
    
public:
    SceneGraphModel(QObject *parent = 0);
    virtual ~SceneGraphModel();

    void SetScene(EditorScene * newScene);
    virtual void Rebuild();

    void SelectNode(DAVA::SceneNode *node);
    DAVA::SceneNode * GetSelectedNode();

Q_SIGNALS:
    
    void SceneNodeSelected(DAVA::SceneNode *node);
    
protected:
    
    void SelectNode(DAVA::SceneNode *node, bool selectAtGraph);
    
    virtual void SelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

    void AddNodeToTree(GraphItem *parent, DAVA::SceneNode *node);

    
protected:

    EditorScene *scene;
    DAVA::SceneNode *selectedNode;
};

#endif // __SCENE_GRAPH_MODEL_H__
