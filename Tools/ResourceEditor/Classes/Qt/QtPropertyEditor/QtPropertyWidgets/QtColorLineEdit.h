#ifndef __QT_BUTTON_LINE_EDIT_H__
#define __QT_BUTTON_LINE_EDIT_H__

#include <QWidget>
#include <QLineEdit>
#include <QToolButton>
#include <QAbstractItemDelegate>

class QtColorLineEdit : public QLineEdit
{
	Q_OBJECT

public:
	QtColorLineEdit(QWidget * parent);

	void SetColor(const QColor &color);
	QColor GetColor() const;

protected:
	QColor curColor;

protected slots:
	void EditFinished();
};

#endif
