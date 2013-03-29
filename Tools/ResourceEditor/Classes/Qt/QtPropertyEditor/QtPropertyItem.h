#ifndef __QT_PROPERTY_ITEM_H__
#define __QT_PROPERTY_ITEM_H__

#include <QStandardItem>

class QtPropertyData;

Q_DECLARE_METATYPE(QtPropertyData *);

class QtPropertyItem : public QObject, public QStandardItem
{
	Q_OBJECT;

public:
	enum PropertyItemDataRole
	{
		PropertyDataRole = Qt::UserRole,
	};

	QtPropertyItem();
	QtPropertyItem(QtPropertyData* data, QtPropertyItem *name);
	QtPropertyItem(const QVariant &value);
	~QtPropertyItem();

	QtPropertyData* GetPropertyData() const;

	int	type() const;
	QVariant data(int role) const;
	void setData(const QVariant & value, int role);
	
protected:
	QtPropertyData* itemData;
	QtPropertyItem *parentName;

	bool itemDataDeleteByParent;

	void ChildAdd(const QString &key, QtPropertyData* data);
	void ChildRemove(QtPropertyData* data);

	void ApplyDataFlags();
	void ApplyNameStyle();

protected slots:
	void DataChildAdded(const QString &key, QtPropertyData *data);
	void DataChildRemoving(const QString &key, QtPropertyData *data);
};

#endif // __QT_PROPERTY_ITEM_H__
