#ifndef __QT_PROPERTY_VIEW_H__
#define __QT_PROPERTY_VIEW_H__

#include <QTreeView>

class QtPropertyItem;
class QtPropertyData;
class QtPropertyModel;
class QtPropertyItemDelegate;

class QtPropertyEditor : public QTreeView
{
	Q_OBJECT

public:
	QtPropertyEditor(QWidget *parent = 0);
	~QtPropertyEditor();

	QtPropertyItem* AppendPropertyHeader(const QString &name, QtPropertyItem* parent = NULL);
	QtPropertyItem* AppendProperty(const QString &name, QtPropertyData* data, QtPropertyItem* parent = NULL);

	void RemoveProperty(QtPropertyItem* item);
	void RemovePropertyAll();

protected:
	QtPropertyModel *curModel;
	QtPropertyItemDelegate *curItemDelegate;
};

#endif // __QT_PROPERTY_VIEW_H__
