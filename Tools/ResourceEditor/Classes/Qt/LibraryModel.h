#ifndef __LIBRARY_MODEL_H__
#define __LIBRARY_MODEL_H__

#include <QFileSystemModel>


class QTreeView;
class FileSelectionModel;
class LibraryModel : public QFileSystemModel
{
    Q_OBJECT
    
public:
    LibraryModel(QObject *parent = 0);
    virtual ~LibraryModel();

    void Activate(QTreeView *view);
    void Deactivate();
    
    void Reload();
    
protected:

    FileSelectionModel *fileSelectionModel;
    QTreeView *attachedTreeView;
};

#endif // __GRAPH_MODEL_H__
