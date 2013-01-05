#ifndef __QT_PROPERTY_DATA_INTROSPECTION_H__
#define __QT_PROPERTY_DATA_INTROSPECTION_H__

#include "Base/Introspection.h"
#include "QtPropertyEditor/QtPropertyData.h"

class QtPropertyDataIntrospection : public QtPropertyData
{
public:
	QtPropertyDataIntrospection(void *object, const DAVA::IntrospectionMember *member);
	virtual ~QtPropertyDataIntrospection();

protected:
	void *object;
	const DAVA::IntrospectionMember *member;

	virtual QVariant GetValueInternal();
	virtual void SetValueInternal(const QVariant &value);
	virtual void ChildChanged(const QString &key, QtPropertyData *data);
	virtual void ChildUpdate();

private:
	void SubPropertiesCreate();
	void SubPropertiesUpdate();

};

#endif // __QT_PROPERTY_DATA_INTROSPECTION_H__
