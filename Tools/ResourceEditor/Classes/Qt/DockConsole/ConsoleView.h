#ifndef __QT_CONSOLE_VIEW_H__
#define __QT_CONSOLE_VIEW_H__

#include <QPlainTextEdit>
#include "Console.h"

class ConsoleView : public QPlainTextEdit
{
	Q_OBJECT

public:
	ConsoleView(QWidget *parent = 0);
	~ConsoleView();

protected:
	size_t lineCount;

protected slots:
	void OnClear();
	void OnEcho();
};

#endif // __QT_CONSOLE_VIEW_H__
