#include "QSceneGraphTreeView.h"

#include "../SceneEditor/SceneEditorScreenMain.h"


#include <QKeyEvent>

QSceneGraphTreeView::QSceneGraphTreeView(QWidget *parent)
    :   QTreeView(parent)
{
}

QSceneGraphTreeView::~QSceneGraphTreeView()
{
}


void QSceneGraphTreeView::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_X)
    {
        SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
        if(screen)
        {
            screen->ProcessIsSolidChanging();
        }
    }
    
    QTreeView::keyPressEvent(event);
}

