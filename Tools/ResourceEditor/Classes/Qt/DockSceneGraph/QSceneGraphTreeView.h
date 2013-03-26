#ifndef __QSCENE_GRAPH_TREE_VIEW_H__
#define __QSCENE_GRAPH_TREE_VIEW_H__

#include <QTreeView>
#include <QMenu>
#include "DAVAEngine.h"

class SceneGraphModel;
class SceneData;
class EditorScene;
class Command;

class QSceneGraphTreeView : public QTreeView
{
    Q_OBJECT
    
public:
   explicit QSceneGraphTreeView(QWidget *parent = 0);
   ~QSceneGraphTreeView();

protected slots:
	// This signal is called by the model when selection in Scene Graph changed.
	void OnSceneNodeSelectedInGraph(DAVA::Entity *node);

	// Called by Scene Data Manager when scene is Created/Released.
	void OnSceneCreated(SceneData* scene);
	void OnSceneReleased(SceneData* scene);

	void OnSceneActivated(SceneData *scene);
	void OnSceneDeactivated(SceneData *scene);

	// Called by Scene Data manager when Scene Graph needs to be updated in a some way.
	void OnSceneGraphNeedSetScene(SceneData *sceneData, EditorScene *scene);
	void OnSceneGraphNeedSelectNode(SceneData *sceneData, DAVA::Entity* node);
	
	// Called by Scene Data Manager when the Scene Graph needs to be rebuilt
	// for the particular node or for the whole graph.
	void OnSceneGraphNeedRebuildNode(DAVA::Entity* node);
	void OnSceneGraphNeedRebuild();

	void OnSceneGraphNeedRefreshLayer(DAVA::ParticleLayer* layer);
	
	// Called by us when the Context Menu is requested.
	void OnSceneGraphContextMenuRequested(const QPoint &point);

	// Called by us when the Context Menu is triggered.
	void SceneGraphMenuTriggered(QAction *action);

protected:
	// Connect/disconnect to signals.
	void ConnectToSignals();
	void DisconnectFromSignals();

	// Support for Context Menu.
	void ShowSceneGraphMenu(const QModelIndex &index, const QPoint &point);
	void AddActionToMenu(QMenu *menu, const QString &actionTitle, Command *command);

	// Context Menu Action Support.
	void ProcessContextMenuAction(QAction *action);

	// Our model.
    SceneGraphModel *sceneGraphModel;
	
    void keyPressEvent(QKeyEvent *event);
	void mouseDoubleClickEvent(QMouseEvent *event);
};

#endif //#ifndef __QSCENE_GRAPH_TREE_VIEW_H__

