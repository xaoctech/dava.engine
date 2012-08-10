#include "LibraryModel.h"


#include "FileSelectionModel.h"
#include <QTreeView>

#include "../SceneEditor/EditorSettings.h"

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
    setNameFilterDisables(true);
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
    setRootPath(QString(EditorSettings::Instance()->GetDataSourcePath().c_str()));
    
    if(attachedTreeView)
    {
        attachedTreeView->setRootIndex(index(QString(EditorSettings::Instance()->GetDataSourcePath().c_str())));
    }
}

FileSelectionModel * LibraryModel::GetSelectionModel()
{
    return fileSelectionModel;
}

