#include "DockSceneTree/SceneTree.h"
#include <QBoxLayout>

SceneTree::SceneTree(QWidget *parent /*= 0*/)
	: QTreeView(parent)
	, skipTreeSelectionProcessing(false)
{
	treeModel = new SceneTreeModel();
	setModel(treeModel);

	setDragDropMode(QAbstractItemView::InternalMove);
	setDragEnabled(true);
	setAcceptDrops(true);
	setDropIndicatorShown(true);

	// scene signals
	QObject::connect(SceneSignals::Instance(), SIGNAL(Activated(SceneEditorProxy *)), this, SLOT(SceneActivated(SceneEditorProxy *)));
	QObject::connect(SceneSignals::Instance(), SIGNAL(Deactivated(SceneEditorProxy *)), this, SLOT(SceneDeactivated(SceneEditorProxy *)));
	QObject::connect(SceneSignals::Instance(), SIGNAL(Selected(SceneEditorProxy *, DAVA::Entity *)), this, SLOT(EntitySelected(SceneEditorProxy *, DAVA::Entity *)));
	QObject::connect(SceneSignals::Instance(), SIGNAL(Deselected(SceneEditorProxy *, DAVA::Entity *)), this, SLOT(EntityDeselected(SceneEditorProxy *, DAVA::Entity *)));

	// this widget signals
	QObject::connect(selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), this, SLOT(TreeSelectionChanged(const QItemSelection &, const QItemSelection &)));
}

SceneTree::~SceneTree()
{

}

void SceneTree::SceneActivated(SceneEditorProxy *scene)
{
	treeModel->SetScene(scene);
}

void SceneTree::SceneDeactivated(SceneEditorProxy *scene)
{
	treeModel->SetScene(NULL);
}

void SceneTree::EntitySelected(SceneEditorProxy *scene, DAVA::Entity *entity)
{
	if(!skipTreeSelectionProcessing)
	{
		skipTreeSelectionProcessing = true;

		if(scene == treeModel->GetScene())
		{
			QModelIndex index = treeModel->GetEntityIndex(entity);

			if(index.isValid())
			{
				selectionModel()->select(index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
				scrollTo(index);
			}
		}
		else
		{
			selectionModel()->clear();
		}

		skipTreeSelectionProcessing = false;
	}
}

void SceneTree::EntityDeselected(SceneEditorProxy *scene, DAVA::Entity *entity)
{
	if(!skipTreeSelectionProcessing)
	{
		skipTreeSelectionProcessing = true;

		if(scene == treeModel->GetScene())
		{
			QModelIndex index = treeModel->GetEntityIndex(entity);

			if(index.isValid())
			{
				selectionModel()->select(index, QItemSelectionModel::Deselect | QItemSelectionModel::Rows);
			}
		}
		else
		{
			selectionModel()->clear();
		}

		skipTreeSelectionProcessing = false;
	}
}

void SceneTree::TreeSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected)
{
	if(!skipTreeSelectionProcessing)
	{
		skipTreeSelectionProcessing = true;

		SceneEditorProxy* curScene = treeModel->GetScene();
		if(NULL != curScene)
		{
			// deselect items in scene
			QModelIndexList indexList = deselected.indexes();
			for (int i = 0; i < indexList.size(); ++i)
			{
				DAVA::Entity *entity = treeModel->GetEntity(indexList[i]);
				curScene->selectionSystem->RemSelection(entity);
			}

			// select items in scene
			indexList = selected.indexes();
			for (int i = 0; i < indexList.size(); ++i)
			{
				DAVA::Entity *entity = treeModel->GetEntity(indexList[i]);
				curScene->selectionSystem->AddSelection(entity);
			}
		}

		skipTreeSelectionProcessing = false;
	}
}
