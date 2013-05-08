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

	QPair<QtPropertyItem*, QtPropertyItem*> AppendProperty(const QString &name, QtPropertyData* data, QtPropertyItem* parent = NULL);

	void RemoveProperty(QtPropertyItem* item);
	void RemovePropertyAll();

	void Expand(QtPropertyItem *);

signals:
	void PropertyChanged(const QString &name, QtPropertyData *data);

protected:
	QtPropertyModel *curModel;
	QtPropertyItemDelegate *curItemDelegate;

protected slots:
	void ItemClicked(const QModelIndex &);
};

#endif // __QT_PROPERTY_VIEW_H__
