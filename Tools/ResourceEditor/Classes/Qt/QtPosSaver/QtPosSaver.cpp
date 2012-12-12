#include "QtPosSaver/QtPosSaver.h"

bool QtPosSaver::settingsArchiveIsLoaded = false;
DAVA::KeyedArchive QtPosSaver::settingsArchive;

QtPosSaver::QtPosSaver()
	: attachedWidget(NULL)
{
	if(!settingsArchiveIsLoaded)
	{
		settingsArchiveIsLoaded = true;
		settingsArchive.Load("~doc:/ResourceEditorPos.archive");
	}
	else
	{
		settingsArchive.Retain();
	}
}

QtPosSaver::~QtPosSaver()
{
	if(settingsArchiveIsLoaded)	
	{
		if(NULL != attachedWidget && !attachedWidgetName.isEmpty())
		{
			SaveGeometry(attachedWidget);

			/*
			DAVA::String key_x = attachedWidgetName + "_x";
			DAVA::String key_y = attachedWidgetName + "_y";
			DAVA::String key_w = attachedWidgetName + "_w";
			DAVA::String key_h = attachedWidgetName + "_h";
			DAVA::String key_m = attachedWidgetName + "_m";

			settingsArchive.SetBool(key_m, attachedWidget->isMaximized());

			if(!attachedWidget->isMaximized())
			{
				settingsArchive.SetInt32(key_x, attachedWidget->pos().x());
				settingsArchive.SetInt32(key_y, attachedWidget->pos().y());
				settingsArchive.SetInt32(key_w, attachedWidget->width());
				settingsArchive.SetInt32(key_h, attachedWidget->height());
			}
			*/
		}

		if(1 == settingsArchive.GetRetainCount())
		{
			settingsArchive.Save("~doc:/ResourceEditorPos.archive");
		}
		else
		{
			settingsArchive.Release();
		}
	}
}

void QtPosSaver::Attach(QWidget *widget)
{
	attachedWidget = widget;

	if(NULL != attachedWidget)
	{
		attachedWidgetName = attachedWidget->objectName();
		LoadGeometry(attachedWidget);

		/*
		DAVA::String key_x = attachedWidgetName + "_x";
		DAVA::String key_y = attachedWidgetName + "_y";
		DAVA::String key_w = attachedWidgetName + "_w";
		DAVA::String key_h = attachedWidgetName + "_h";
		DAVA::String key_m = attachedWidgetName + "_m";

		int x = settingsArchive.GetInt32(key_x);
		int y = settingsArchive.GetInt32(key_y);
		int w = settingsArchive.GetInt32(key_w);
		int h = settingsArchive.GetInt32(key_h);
		bool m = settingsArchive.GetBool(key_m);

		if(0 != w && 0 != h)
		{
			attachedWidget->move(x, y);
			attachedWidget->resize(w, h);

			if(m)
			{
				attachedWidget->showMaximized();
			}
		}
		*/
	}
}

void QtPosSaver::SaveGeometry(QWidget *widget)
{
	if(NULL != widget && !attachedWidgetName.isEmpty())
	{
		QString key = attachedWidgetName + "-geometry-" + widget->objectName();
		Save(key, widget->saveGeometry());

		key = attachedWidgetName + "-maximized-" + widget->objectName();
		QByteArray mState(1, (char) widget->isMaximized());
		Save(key, mState);
	}
}

void QtPosSaver::LoadGeometry(QWidget *widget)
{
	if(NULL != widget && !attachedWidgetName.isEmpty())
	{
		QString key = attachedWidgetName + "-maximized-" + widget->objectName();
		QByteArray mState = Load(key);
		if(!mState.isEmpty() && mState.at(0))
		{
			widget->showMaximized();
		}

		key = attachedWidgetName + "-geometry-" + widget->objectName();
		widget->restoreGeometry(Load(key));
	}
}

void QtPosSaver::SaveState(QSplitter *splitter)
{
	if(NULL != splitter && !attachedWidgetName.isEmpty())
	{
		QString key = attachedWidgetName + "-splitter-" + splitter->objectName();
		Save(key, splitter->saveState());
	}
}

void QtPosSaver::LoadState(QSplitter *splitter)
{
	if(NULL != splitter && !attachedWidgetName.isEmpty())
	{
		QString key = attachedWidgetName + "-splitter-" + splitter->objectName();
		splitter->restoreState(Load(key));
	}
}

void QtPosSaver::SaveState(QMainWindow *mainwindow)
{
	if(NULL != mainwindow && !attachedWidgetName.isEmpty())
	{
		QString key = attachedWidgetName + "-mainwindow-" + mainwindow->objectName();
		Save(key, mainwindow->saveState());
	}
}

void QtPosSaver::LoadState(QMainWindow *mainwindow)
{
	if(NULL != mainwindow && !attachedWidgetName.isEmpty())
	{
		QString key = attachedWidgetName + "-mainwindow-" + mainwindow->objectName();
		mainwindow->restoreState(Load(key));
	}
}

void QtPosSaver::Save(const QString &key, QByteArray &data)
{
	if(settingsArchiveIsLoaded && !key.isEmpty() && !data.isEmpty())
	{
		settingsArchive.SetByteArray(key.toStdString(), (const DAVA::uint8 *) data.constData(), data.size());
	}
}

QByteArray QtPosSaver::Load(const QString &key)
{
	QByteArray data;

	if(settingsArchiveIsLoaded && !key.isEmpty())
	{
		int sz = settingsArchive.GetByteArraySize(key.toStdString());
		const DAVA::uint8 *dt = settingsArchive.GetByteArray(key.toStdString());

		if(NULL != dt)
		{
			data.append((const char *) dt, sz);
		}
	}

	return data;
}