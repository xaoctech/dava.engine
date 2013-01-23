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

	void SetItemDelegatePtr(const QAbstractItemDelegate* deleg);

protected:
	QColor curColor;
	QToolButton *toolButton;
	const QAbstractItemDelegate* deleg;

protected slots:
	void ToolButtonClicked();
	void EditFinished();
};

#endif
