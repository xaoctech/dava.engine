#include "LibraryModel.h"


#include "FileSelectionModel.h"
#include <QTreeView>

#include "../SceneEditor/EditorSettings.h"

#include "QtUtils.h"

#include "DAVAEngine.h"

using namespace DAVA;

LibraryModel::LibraryModel(QObject *parent)
    :   QFileSystemModel(parent)
    //,   attachedTreeView(NULL)
{
    //fileSelectionModel = new FileSelectionModel(this);
    
    QStringList nameFilters;
    nameFilters << QString("*.sc2");
    nameFilters << QString("*.dae");

    setNameFilters(nameFilters);
    setNameFilterDisables(false);
}

LibraryModel::~LibraryModel()
{
    //SafeDelete(fileSelectionModel);
}

/*
void LibraryModel::Deactivate()
{
    attachedTreeView = NULL;
}

void LibraryModel::Activate(QTreeView *view)
{
    DVASSERT((NULL == attachedTreeView) && "View must be deactivated")
    
    attachedTreeView = view;
    
    attachedTreeView->setModel(this);
    attachedTreeView->setSelectionModel(fileSelectionModel);
    
	int32 count = columnCount();
    for(int32 i = 1; i < count; ++i)
    { //TODO: Maybe we will use context menu to enable/disable columns
        attachedTreeView->setColumnHidden(i, true);
    }
    
    SetLibraryPath();
}
*/

void LibraryModel::SetLibraryPath(const QString &path)
{
	setRootPath(QDir(path).canonicalPath());
}

/*
FileSelectionModel * LibraryModel::GetSelectionModel()
{
    return fileSelectionModel;
}
*/

QVariant LibraryModel::data(const QModelIndex &index, int role) const
{
    if(index.isValid() && (Qt::TextColorRole == role))
    {
        QFileInfo info = fileInfo(index);
        
        if(info.isFile())
        {
            if(0 == CompareCaseInsensitive(".sc2", info.suffix().toStdString()))
            {
                return QColor(158, 0, 0);
            }
            return QColor(Qt::black);
        }
    }
    
    return QFileSystemModel::data(index, role);
}

/*
bool LibraryModel::SelectFile(const QString &path)
{
	if(!attachedTreeView)
		return false;

	QString filePathname = QStylePathname(pathname); 
	const QModelIndex fileIndex = index(filePathname);
	attachedTreeView->setCurrentIndex(fileIndex);
	attachedTreeView->scrollTo(fileIndex);

	return true;
}
*/

