#include "LibraryModel.h"


#include "FileSelectionModel.h"
#include <QTreeView>

#include "../SceneEditor/EditorSettings.h"

#include "QtUtils.h"

#include "DAVAEngine.h"

using namespace DAVA;

LibraryModel::LibraryModel(QObject *parent)
    :   QFileSystemModel(parent)
    ,   attachedTreeView(NULL)
{
    fileSelectionModel = new FileSelectionModel(this);
    
    QStringList nameFilters;
    nameFilters << QString("*.sc2");
    nameFilters << QString("*.dae");
    setNameFilters(nameFilters);
    setNameFilterDisables(false);
}

LibraryModel::~LibraryModel()
{
    SafeDelete(fileSelectionModel);
}


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
    
    Reload();
}

void LibraryModel::Reload()
{
	QString datasourcePathname(EditorSettings::Instance()->GetDataSourcePath().c_str());
	QDir datasosurceFolder(datasourcePathname);
	QString rootPath = datasosurceFolder.canonicalPath(); 

	setRootPath(rootPath);
    if(attachedTreeView)
    {
        attachedTreeView->setRootIndex(index(rootPath));
    }
}

FileSelectionModel * LibraryModel::GetSelectionModel()
{
    return fileSelectionModel;
}

QVariant LibraryModel::data(const QModelIndex &index, int role) const
{
    if(index.isValid() && (Qt::TextColorRole == role))
    {
        QFileInfo info = fileInfo(index);
        
        if(info.isFile())
        {
            String extension = FileSystem::Instance()->GetExtension(QSTRING_TO_DAVASTRING(info.fileName()));
            if(0 == CompareStrings(".sc2", extension))
            {
                return QColor(158, 0, 0);
            }
            return QColor(Qt::black);
        }
    }
    
    return QFileSystemModel::data(index, role);
}


