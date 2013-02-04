#ifndef __LIBRARY_MODEL_H__
#define __LIBRARY_MODEL_H__

#include <QFileSystemModel>
#include <QString>
#include "DAVAEngine.h"

class QTreeView;
class FileSelectionModel;

struct ExtensionToColorMap
{
	QString extension;
	QColor color;
};

class LibraryModel : public QFileSystemModel
{
    Q_OBJECT
    
public:
    LibraryModel(QObject *parent = 0);
    virtual ~LibraryModel();

	void SetFileNameFilters(bool showDAEFiles, bool showSC2Files);

    void SetLibraryPath(const QString &path);
	// bool SelectFile(const QString &path);
    
    virtual QVariant data(const QModelIndex &index, int role) const;
	
protected:
	QVariant GetColorForExtension(const QString& extension, const ExtensionToColorMap* colorMap,
								const QModelIndex &index, int role) const;

private:
	static ExtensionToColorMap extensionToBackgroundColorMap[];
};

#endif // __GRAPH_MODEL_H__
