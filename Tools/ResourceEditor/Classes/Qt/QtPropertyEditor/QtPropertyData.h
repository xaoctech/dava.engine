#ifndef __QT_PROPERTY_DATA_H__
#define __QT_PROPERTY_DATA_H__

#include <QStyledItemDelegate>

class QtPropertyData
{
public:
	QtPropertyData() { }
	virtual ~QtPropertyData() { }

	virtual QVariant GetValue() = 0;
	virtual void SetValue(const QVariant &data) = 0;

	QWidget *CreateEditor(QWidget *parent, const QStyleOptionViewItem& option) { return NULL; }
};

#endif // __QT_PROPERTY_DATA_H__
