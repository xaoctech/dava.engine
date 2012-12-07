#ifndef __LIBRARY_VIEW_H__
#define __LIBRARY_VIEW_H__

#include <QTreeView>
#include "Scene/SceneDataManager.h"
#include "Main/LibraryModel.h"

class LibraryView : public QTreeView
{
	Q_OBJECT

public:
	LibraryView(QWidget *parent = 0);
	~LibraryView();

public slots:
	void sceneActivated(SceneData *scene);
	void sceneChanged(SceneData *scene);
	void sceneReleased(SceneData *scene);
	void sceneNodeSelected(SceneData *scene, DAVA::SceneNode *node);

private:
	LibraryModel *libModel;
};

#endif // __LIBRARY_VIEW_H__ 
