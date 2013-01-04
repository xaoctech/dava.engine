#include <QTreeView>
#include "../SceneEditor/EditorSettings.h"
#include "Main/QtUtils.h"
#include "DockLibrary/LibraryModel.h"
#include "DAVAEngine.h"

using namespace DAVA;

LibraryModel::LibraryModel(QObject *parent)
    :   QFileSystemModel(parent)
{
    QStringList nameFilters;
    nameFilters << QString("*.sc2");
    nameFilters << QString("*.dae");

    setNameFilters(nameFilters);
    setNameFilterDisables(false);
}

LibraryModel::~LibraryModel()
{ }

void LibraryModel::SetLibraryPath(const QString &path)
{
	setRootPath(QDir(path).canonicalPath());
}

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
