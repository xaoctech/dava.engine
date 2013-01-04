#ifndef __QT_PROPERTY_DATA_INTROSPECTION_H__
#define __QT_PROPERTY_DATA_INTROSPECTION_H__

#include "Base/Introspection.h"
#include "QtPropertyEditor/QtPropertyData.h"

class QtPropertyDataIntrospection : public QtPropertyData
{
public:
	QtPropertyDataIntrospection(void *object, const DAVA::IntrospectionMember *member);
	virtual ~QtPropertyDataIntrospection();

	virtual QVariant GetValue();
	virtual void SetValue(const QVariant &value);

public:
	void *object;
	const DAVA::IntrospectionMember *member;
};

#endif // __QT_PROPERTY_DATA_INTROSPECTION_H__
