#ifndef __QT_PROPERTY_DATA_COMMON_H__
#define __QT_PROPERTY_DATA_COMMON_H__

#include "Base/Introspection.h"
#include "QtPropertyEditor/QtPropertyData.h"

class QtPropertyDataCommon : public QtPropertyData
{
public:
	QtPropertyDataCommon(const QVariant& value);
	virtual ~QtPropertyDataCommon();

	virtual QVariant GetValue();
	virtual void SetValue(const QVariant &value);

public:
	QVariant value;
};

#endif // __QT_PROPERTY_DATA_INTROSPECTION_H__
