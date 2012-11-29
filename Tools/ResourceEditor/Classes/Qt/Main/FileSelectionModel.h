#ifndef __FILE_SELECTION_MODEL_H__
#define __FILE_SELECTION_MODEL_H__

#include <QItemSelectionModel>


class QFileSystemModel;
class FileSelectionModel: public QItemSelectionModel
{
    Q_OBJECT
    
public:
    FileSelectionModel(QFileSystemModel *model = 0);
    virtual ~FileSelectionModel();
    
Q_SIGNALS:
    void FileSelected(const QString &filePathname, bool isFile);

protected slots:
    
    virtual void SelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    
protected:

    QFileSystemModel *fileSystemModel;
};

#endif // __FILE_SELECTION_MODEL_H__
