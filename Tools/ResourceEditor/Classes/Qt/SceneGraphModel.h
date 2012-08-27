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

    virtual Qt::ItemFlags flags(const QModelIndex &index) const;

    virtual Qt::DropActions supportedDropActions() const;
    virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());
    virtual bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    
    virtual void MoveItemToParent(GraphItem * movedItem, const QModelIndex &newParentIndex);

    
Q_SIGNALS:
    
    void SceneNodeSelected(DAVA::SceneNode *node);
    
protected:
    
    void SelectNode(DAVA::SceneNode *node, bool selectAtGraph);
    void SelectItem(GraphItem *item);
    
    virtual void SelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

    void AddNodeToTree(GraphItem *parent, DAVA::SceneNode *node);

    
protected:

    EditorScene *scene;
    DAVA::SceneNode *selectedNode;
};

#endif // __SCENE_GRAPH_MODEL_H__
