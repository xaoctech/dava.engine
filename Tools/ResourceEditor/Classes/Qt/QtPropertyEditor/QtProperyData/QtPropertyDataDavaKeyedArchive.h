#ifndef __QT_PROPERTY_DATA_DAVA_KEYEDARCHIVE_H__
#define __QT_PROPERTY_DATA_DAVA_KEYEDARCHIVE_H__

#include "Base/Introspection.h"
#include "QtPropertyEditor/QtPropertyData.h"

class QtPropertyDataDavaKeyedArcive : public QtPropertyData
{
	Q_OBJECT;

public:
	QtPropertyDataDavaKeyedArcive(DAVA::KeyedArchive *archive);
	virtual ~QtPropertyDataDavaKeyedArcive();

protected:
	DAVA::KeyedArchive* curArchive;

	virtual QVariant GetValueInternal();
	virtual void SetValueInternal(const QVariant &value);
	virtual void ChildChanged(const QString &key, QtPropertyData *data);

private:
	void ChildsCreate();

protected slots:
	void AddKeyedArchiveField();
	void RemKeyedArchiveField();

};

#endif // __QT_PROPERTY_DATA_DAVA_KEYEDARCHIVE_H__
