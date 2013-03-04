#ifndef __SCENE_GRAPH_MODEL_H__
#define __SCENE_GRAPH_MODEL_H__

#include "DAVAEngine.h"
#include "GraphModel.h"
#include "ParticlesEditorQT/Helpers/ParticlesEditorSceneModelHelper.h"

#include <QObject>
#include <QMap>
#include <QIcon>

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

	// Get the persistent data for Model Index, needed for save/restore
	// SceneGraphModel expanded state.
	void* GetPersistentDataForModelIndex(const QModelIndex &modelIndex);

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
    void InitDecorationIcons();
	QIcon GetDecorationIcon(DAVA::SceneNode *node) const;

    void SelectNode(DAVA::SceneNode *node, bool selectAtGraph);
    void SelectItem(GraphItem *item, bool needExpand = false);
    
    virtual void SelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

    void AddNodeToTree(GraphItem *parent, DAVA::SceneNode *node, bool partialUpdate = false);
    
    bool LandscapeEditorModeEnabled() const;
    
    // Custom selection handling for Particle Editor.
    bool HandleParticleEditorSelection();
	
	// Tree item checkboxes functionality.
	// Is particular item checkable?
	bool IsItemCheckable(const QModelIndex &index) const;
	
	// Get/set the checked state for the item.
	Qt::CheckState GetItemCheckState(const QModelIndex& index) const;
	void SetItemCheckState(const QModelIndex& index, bool value);
	
	// Get the graph model by Model Index (or NULL if no Graph Model attached).
	GraphItem* GetGraphItemByModelIndex(const QModelIndex& index) const;

	//Nodes which should not be displayed in SceneGraph tree must return false when passed to this function
	bool IsNodeAccepted(DAVA::SceneNode* node);
protected:

    EditorScene *scene;
    DAVA::SceneNode *selectedNode;
    
    DAVA::ParticlesEditorSceneModelHelper particlesEditorSceneModelHelper;
    
    // Selected Scene Graph item for Particle Editor.
    SceneGraphItem* selectedGraphItemForParticleEditor;

	QMap<QString, QIcon> decorationIcons;
};

#endif // __SCENE_GRAPH_MODEL_H__
