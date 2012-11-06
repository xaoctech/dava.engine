#ifndef __QT_POS_SAVER_H__
#define __QT_POS_SAVER_H__

#include <QMainWindow>
#include <QDialog>
#include "DAVAEngine.h"

class QtPosSaver
{
public:
	QtPosSaver();
	virtual ~QtPosSaver();

	void Load(QWidget *widget, const DAVA::String &classKey);

private:
	QWidget *m_widget;
	DAVA::String m_classKey;

	static bool settingsArchiveIsLoaded;
	static DAVA::KeyedArchive settingsArchive;
};

#endif // __QT_POS_SAVER_H__
