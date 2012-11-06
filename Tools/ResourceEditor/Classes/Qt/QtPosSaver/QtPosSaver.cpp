#include "QtPosSaver/QtPosSaver.h"

bool QtPosSaver::settingsArchiveIsLoaded = false;
DAVA::KeyedArchive QtPosSaver::settingsArchive;

QtPosSaver::QtPosSaver()
	: m_widget(NULL)
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
		if(NULL != m_widget && !m_classKey.empty())
		{
			DAVA::String key_x = m_classKey + "_x";
			DAVA::String key_y = m_classKey + "_y";
			DAVA::String key_w = m_classKey + "_w";
			DAVA::String key_h = m_classKey + "_h";
			DAVA::String key_m = m_classKey + "_m";

			settingsArchive.SetBool(key_m, m_widget->isMaximized());

			if(!m_widget->isMaximized())
			{
				settingsArchive.SetInt32(key_x, m_widget->pos().x());
				settingsArchive.SetInt32(key_y, m_widget->pos().y());
				settingsArchive.SetInt32(key_w, m_widget->width());
				settingsArchive.SetInt32(key_h, m_widget->height());
			}
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

void QtPosSaver::Load(QWidget *widget, const DAVA::String &classFileName)
{
	m_widget = widget;

	if(NULL != m_widget)
	{
		m_classKey = classFileName;

		DAVA::String key_x = m_classKey + "_x";
		DAVA::String key_y = m_classKey + "_y";
		DAVA::String key_w = m_classKey + "_w";
		DAVA::String key_h = m_classKey + "_h";
		DAVA::String key_m = m_classKey + "_m";

		int x = settingsArchive.GetInt32(key_x);
		int y = settingsArchive.GetInt32(key_y);
		int w = settingsArchive.GetInt32(key_w);
		int h = settingsArchive.GetInt32(key_h);
		bool m = settingsArchive.GetBool(key_m);

		if(0 != w && 0 != h)
		{
			m_widget->move(x, y);
			m_widget->resize(w, h);

			if(m)
			{
				m_widget->showMaximized();
			}
		}
	}
}
