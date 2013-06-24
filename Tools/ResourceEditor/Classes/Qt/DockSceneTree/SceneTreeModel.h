#ifndef __QT_SCENE_TREE_MODEL_H__
#define __QT_SCENE_TREE_MODEL_H__

#include <QPair>
#include <QStandardItemModel>

#include "Scene/SceneEditorProxy.h"
#include "Qt/DockSceneTree/SceneTreeItem.h"

// framework
#include "Scene3D/Scene.h"

class SceneTreeModel : public QStandardItemModel
{
	Q_OBJECT

public:
	SceneTreeModel(QObject* parent = 0);
	~SceneTreeModel();

	void SetScene(SceneEditorProxy *scene);
	SceneEditorProxy* GetScene() const;

	QModelIndex GetEntityIndex(DAVA::Entity *entity) const;
	DAVA::Entity* GetEntity(const QModelIndex &index) const;

	// drag and drop support
	Qt::DropActions supportedDropActions() const;
	QMimeData *	mimeData(const QModelIndexList & indexes) const;
	QStringList	mimeTypes() const;
	bool dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent);

protected:
	SceneEditorProxy * curScene;
};

#endif // __QT_SCENE_TREE_MODEL_H__
