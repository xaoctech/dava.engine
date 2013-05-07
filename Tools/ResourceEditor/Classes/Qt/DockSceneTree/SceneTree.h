#ifndef __QT_SCENE_TREE_H__
#define __QT_SCENE_TREE_H__

#include <QWidget>
#include <QTreeView>
#include <QTableView>

#include "Scene/SceneSignals.h"
#include "DockSceneTree/SceneTreeModel.h"

class SceneTree : public QTreeView
{
	Q_OBJECT

public:
	SceneTree(QWidget *parent = 0);
	~SceneTree();

protected:
	SceneTreeModel * treeModel;
	bool skipTreeSelectionProcessing;

protected slots:
	void SceneActivated(SceneEditorProxy *scene);
	void SceneDeactivated(SceneEditorProxy *scene);
	void EntitySelected(SceneEditorProxy *scene, DAVA::Entity *entity);
	void EntityDeselected(SceneEditorProxy *scene, DAVA::Entity *entity);

	void TreeSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected);
};

#endif // __QT_SCENE_TREE_H__
