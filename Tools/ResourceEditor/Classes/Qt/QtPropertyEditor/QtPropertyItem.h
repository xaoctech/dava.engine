#ifndef __QT_PROPERTY_ITEM_H__
#define __QT_PROPERTY_ITEM_H__

#include <QStandardItem>

class QtPropertyData;

class QtPropertyItem : public QStandardItem
{
public:
	QtPropertyItem(QtPropertyData* data = NULL);
	~QtPropertyItem();

	void SetPropertyData(QtPropertyData* data);
	QtPropertyData* GetPropertyData() const;

	int	type() const;
	QVariant data(int role = Qt::UserRole + 1) const;
	void setData(const QVariant & value, int role = Qt::UserRole + 1);

protected:
	QtPropertyData* itemData;
};

#endif // __QT_PROPERTY_ITEM_H__
