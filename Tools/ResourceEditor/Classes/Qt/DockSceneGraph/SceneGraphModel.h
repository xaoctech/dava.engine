#ifndef __SCENE_GRAPH_MODEL_H__
#define __SCENE_GRAPH_MODEL_H__

#include "DAVAEngine.h"
#include "GraphModel.h"
#include "ParticlesEditorQT/Helpers/ParticlesEditorSceneModelHelper.h"

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
	EditorScene* GetScene() const {return scene;};

	// Rebuild the model for the appropriate node and for the whole graph.
	void RebuildNode(DAVA::SceneNode* rootNode);
    virtual void Rebuild();

	// Refresh the Particle Editor Layer.
	void RefreshParticlesLayer(DAVA::ParticleLayer* layer);

    void SelectNode(DAVA::SceneNode *node);
    DAVA::SceneNode * GetSelectedNode();

   const DAVA::ParticlesEditorSceneModelHelper& GetParticlesEditorSceneModelHelper() const { return particlesEditorSceneModelHelper; };

    virtual Qt::ItemFlags flags(const QModelIndex &index) const;

    virtual Qt::DropActions supportedDropActions() const;
    virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());
    virtual bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    
    virtual bool MoveItemToParent(GraphItem * movedItem, const QModelIndex &newParentIndex);

    virtual QVariant data(const QModelIndex &index, int role) const;
    
Q_SIGNALS:
    
    void SceneNodeSelected(DAVA::SceneNode *node);
    
protected:
    
    void SelectNode(DAVA::SceneNode *node, bool selectAtGraph);
    void SelectItem(GraphItem *item, bool needExpand = false);
    
    virtual void SelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

    void AddNodeToTree(GraphItem *parent, DAVA::SceneNode *node);
    
    bool LandscapeEditorModeEnabled() const;
    
    // Custom selection handling for Particle Editor.
    bool HandleParticleEditorSelection();

	// Add the Graph Items in a recursive way.
	void AddGraphItemsRecursive(GraphItem* rootItem, SceneNode* rootNode);

protected:

    EditorScene *scene;
    DAVA::SceneNode *selectedNode;
    
    DAVA::ParticlesEditorSceneModelHelper particlesEditorSceneModelHelper;
    
    // Selectted Scene Graph item for Particle Editor.
    SceneGraphItem* selectedGraphItemForParticleEditor;
};

#endif // __SCENE_GRAPH_MODEL_H__
