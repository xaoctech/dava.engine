#include "FileSelectionModel.h"
#include "GUIActionHandler.h"

#include <QFileSystemModel>
#include <QFileInfo>


FileSelectionModel::FileSelectionModel(QFileSystemModel *model)
	:	QItemSelectionModel(model)
    ,   fileSystemModel(model)
{
    connect(this, SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), this, SLOT(SelectionChanged(const QItemSelection &, const QItemSelection &)));
    connect(this, SIGNAL(FileSelected(const QString &, bool)), GUIActionHandler::Instance(), SLOT(FileSelected(const QString &, bool)));
}

FileSelectionModel::~FileSelectionModel()
{
}

void FileSelectionModel::SelectionChanged(const QItemSelection &selected, const QItemSelection &)
{
    int32 selectedSize = selected.size();
    DVASSERT((selectedSize <= 1) && "Wrong count of selected items");
    
    if(0 < selectedSize)
    {
        QItemSelectionRange selectedRange = selected.at(0);
        QFileInfo fileInfo = fileSystemModel->fileInfo(selectedRange.topLeft());

        emit FileSelected(fileInfo.filePath(), fileInfo.isFile());
    }
}
