#ifndef __QSCENE_GRAPH_TREE_VIEW_H__
#define __QSCENE_GRAPH_TREE_VIEW_H__

#include <QTreeView>

class LibraryModel;
class QSceneGraphTreeView : public QTreeView
{
    Q_OBJECT
    
public:
   explicit QSceneGraphTreeView(QWidget *parent = 0);
   ~QSceneGraphTreeView();
    
protected:
    
    void keyPressEvent(QKeyEvent *event);
};

#endif //#ifndef __QSCENE_GRAPH_TREE_VIEW_H__

