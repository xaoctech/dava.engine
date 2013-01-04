#ifndef __QT_PROPERTY_MODEL_H__
#define __QT_PROPERTY_MODEL_H__

#include <QStandardItemModel>
#include "Base/Introspection.h"

#include "QtPropertyEditor/QtPropertyItem.h"
#include "QtPropertyEditor/QtPropertyData.h"

class QtPropertyModel : public QStandardItemModel
{
	Q_OBJECT

public:
	QtPropertyModel(QObject* parent = 0);
	~QtPropertyModel();

	QtPropertyItem* AppendPropertyHeader(const QString &name, QtPropertyItem* parent = NULL);
	QtPropertyItem* AppendProperty(const QString &name, QtPropertyData* data, QtPropertyItem* parent = NULL);

	void RemoveProperty(QtPropertyItem* item);
	void RemovePropertyAll();
};

#endif // __QT_PROPERTY_MODEL_H__
