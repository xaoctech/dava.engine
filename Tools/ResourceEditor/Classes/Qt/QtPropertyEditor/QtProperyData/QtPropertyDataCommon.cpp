#include "DAVAEngine.h"
#include "QtPropertyEditor/QtProperyData/QtPropertyDataCommon.h"

QtPropertyDataCommon::QtPropertyDataCommon(const QVariant& _value)
	: value(_value)
{ }

QtPropertyDataCommon::~QtPropertyDataCommon()
{ }

QVariant QtPropertyDataCommon::GetValue()
{
	return value;
}

void QtPropertyDataCommon::SetValue(const QVariant &_value)
{
	value = _value;
}
