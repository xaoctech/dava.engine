#ifndef __LIBRARY_MODEL_H__
#define __LIBRARY_MODEL_H__

#include <QFileSystemModel>
#include <QString>
#include "DAVAEngine.h"

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
    
    FileSelectionModel *GetSelectionModel();
    
    virtual QVariant data(const QModelIndex &index, int role) const;
    
	bool SelectFile(const DAVA::String &pathname);

protected:

	QString QStylePathname(const DAVA::String &pathname);

    FileSelectionModel *fileSelectionModel;
    QTreeView *attachedTreeView;
};

#endif // __GRAPH_MODEL_H__
