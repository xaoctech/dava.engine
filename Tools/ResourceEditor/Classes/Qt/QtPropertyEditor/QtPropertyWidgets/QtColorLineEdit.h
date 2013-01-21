#ifndef __QT_BUTTON_LINE_EDIT_H__
#define __QT_BUTTON_LINE_EDIT_H__

#include <QWidget>
#include <QLineEdit>
#include <QToolButton>

class QtColorLineEdit : public QLineEdit
{
	Q_OBJECT

public:
	QtColorLineEdit(QWidget * parent);
	void SetColor(const QColor &color);
	QColor GetColor() const;

protected:
	QColor curColor;
	QToolButton *toolButton;

protected slots:
	void ToolButtonClicked();
	void EditFinished();
};

#endif
