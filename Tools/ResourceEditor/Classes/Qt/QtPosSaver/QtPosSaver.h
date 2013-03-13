#ifndef __QT_POS_SAVER_H__
#define __QT_POS_SAVER_H__

#include <QMainWindow>
#include <QDialog>
#include <QSplitter>
#include <QMainWindow>
#include <QTreeView>
#include "DAVAEngine.h"

class QtPosSaver
{
public:
	QtPosSaver();
	virtual ~QtPosSaver();

	void Attach(QWidget *widget, const QString &name = QString());

	void SaveGeometry(QWidget *widget);
	void LoadGeometry(QWidget *widget);

	void SaveState(QSplitter *splitter);
	void LoadState(QSplitter *splitter);

	void SaveState(QMainWindow *mainwindow);
	void LoadState(QMainWindow *mainwindow);

	void SaveValue(const QString &key, const DAVA::VariantType &value);
	DAVA::VariantType LoadValue(const QString &key);

protected:
	QWidget *attachedWidget;
	QString attachedWidgetName;

	void Save(const QString &key, const QByteArray &data);
	QByteArray Load(const QString &key);

private:
	static bool settingsArchiveIsLoaded;
	static DAVA::KeyedArchive settingsArchive;
};

#endif // __QT_POS_SAVER_H__
