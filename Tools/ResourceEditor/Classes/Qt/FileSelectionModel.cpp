#include "FileSelectionModel.h"

#include <QFileSystemModel>
#include <QFileInfo>

#include "DAVAEngine.h"

using namespace DAVA;

FileSelectionModel::FileSelectionModel(QFileSystemModel *model)
	:	QItemSelectionModel(model)
    ,   fileSystemModel(model)
{
    connect(this, SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), this, SLOT(SelectionChanged(const QItemSelection &, const QItemSelection &)));
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
