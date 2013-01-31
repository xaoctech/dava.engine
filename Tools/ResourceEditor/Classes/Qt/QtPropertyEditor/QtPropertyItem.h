#ifndef __QT_PROPERTY_ITEM_H__
#define __QT_PROPERTY_ITEM_H__

#include <QStandardItem>

class QtPropertyData;

class QtPropertyItem : public QStandardItem
{
public:
	QtPropertyItem();
	QtPropertyItem(QtPropertyData* data, QtPropertyItem *name);
	QtPropertyItem(const QVariant &value);
	~QtPropertyItem();

	QtPropertyData* GetPropertyData() const;

	int	type() const;
	QVariant data(int role = Qt::UserRole + 1) const;
	void setData(const QVariant & value, int role = Qt::UserRole + 1);

protected:
	QtPropertyData* itemData;
	bool itemDataDeleteByParent;

	void ApplyDataFlags();
	void ApplyNameStyle(QtPropertyItem *name);
};

#endif // __QT_PROPERTY_ITEM_H__
