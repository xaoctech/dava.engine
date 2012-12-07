#include "DockLibrary/LibraryView.h"

LibraryView::LibraryView(QWidget *parent /* = 0 */)
	: QTreeView(parent)
{
	libModel = new LibraryModel();

	// global scene manager signals
	QObject::connect(SceneDataManager::Instance(), SIGNAL(SceneActivated(SceneData *)), this, SLOT(sceneActivated(SceneData *)));
	QObject::connect(SceneDataManager::Instance(), SIGNAL(SceneChanged(SceneData *)), this, SLOT(sceneChanged(SceneData *)));
	QObject::connect(SceneDataManager::Instance(), SIGNAL(SceneReleased(SceneData *)), this, SLOT(sceneReleased(SceneData *)));
	QObject::connect(SceneDataManager::Instance(), SIGNAL(SceneNodeSelected(SceneData *, DAVA::SceneNode *)), this, SLOT(sceneNodeSelected(SceneData *, DAVA::SceneNode *)));
}

LibraryView::~LibraryView()
{

}

void LibraryView::sceneActivated(SceneData *scene)
{
	libModel->Reload();
}

void LibraryView::sceneChanged(SceneData *scene)
{
	libModel->Reload();
}

void LibraryView::sceneReleased(SceneData *scene)
{

}

void LibraryView::sceneNodeSelected(SceneData *scene, DAVA::SceneNode *node)
{

}
