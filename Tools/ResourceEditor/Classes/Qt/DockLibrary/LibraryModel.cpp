#include <QTreeView>
#include "../SceneEditor/EditorSettings.h"
#include "Main/QtUtils.h"
#include "DockLibrary/LibraryModel.h"
#include "DAVAEngine.h"

using namespace DAVA;

// If you need to specify different colors for different files extensions.
// expand this list.
ExtensionToColorMap LibraryModel::extensionToColorMap[] =
{
	{"sc2", QColor(158, 0, 0) },
	{"dae", QColor(222, 239, 254) }
};

LibraryModel::LibraryModel(QObject *parent)
    :   QFileSystemModel(parent)
{
	SetFileNameFilters(true, true);
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
			return GetColorForExtension(info.suffix());
        }
    }
    
    return QFileSystemModel::data(index, role);
}

void LibraryModel::SetFileNameFilters(bool showDAEFiles, bool showSC2Files)
{
	QStringList nameFilters;
	if (showDAEFiles)
	{
	    nameFilters << QString("*.dae");
	}
	if (showSC2Files)
	{
		nameFilters << QString("*.sc2");
	}

	QDir::Filters filterForNoneSelected = QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Dirs | QDir::Drives;
	if (nameFilters.empty())
	{
		// Show directories only in case nothing is selected.
		this->setFilter(filterForNoneSelected);
	}
	else
	{
		// Show directories and files in case file tyoes are selected.
		this->setFilter(filterForNoneSelected | QDir::Files);
		setNameFilters(nameFilters);
	}
}

QColor LibraryModel::GetColorForExtension(const QString& extension) const
{
	int32 extensionsCount = sizeof(extensionToColorMap) / sizeof(*extensionToColorMap);
	for (int32 i = 0; i < extensionsCount; i ++)
	{
		if (extensionToColorMap[i].extension.compare(extension,  Qt::CaseInsensitive) == 0)
		{
			return extensionToColorMap[i].color;
		}
	}
	
	return QColor(Qt::black);
}
