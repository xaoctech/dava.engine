#ifndef __GRAPH_TREE_VIEW_H__
#define __GRAPH_TREE_VIEW_H__

#include <QTreeView>

class GraphTreeView : public QTreeView
{
    Q_OBJECT
    
public:
    GraphTreeView(QWidget *parent = 0);
    ~GraphTreeView();
    
};

#endif // __GRAPH_TREE_VIEW_H__
