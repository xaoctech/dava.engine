#include "DAVAEngine.h"
#include "QtPropertyEditor/QtProperyData/QtPropertyDataIntrospection.h"
#include "QtPropertyEditor/QtProperyData/QtPropertyDataDavaVariant.h"

QtPropertyDataIntrospection::QtPropertyDataIntrospection(void *_object, const DAVA::IntrospectionInfo *_info)
	: object(_object)
	, info(_info)
{
	if(NULL != info)
	{
		for(int i = 0; i < info->MembersCount(); ++i)
		{
			QtPropertyDataDavaVariant *childData = new QtPropertyDataDavaVariant(info->Member(i)->Value(object));
			ChildAdd(info->Member(i)->Name(), childData);

			childIndexes.insert(childData, i);
		}
	}

	SetFlags(FLAG_IS_DISABLED);
}

QtPropertyDataIntrospection::~QtPropertyDataIntrospection()
{ }

QVariant QtPropertyDataIntrospection::GetValueInternal()
{
	QVariant v;

	if(NULL != info)
	{
		v = info->Name();
		ChildNeedUpdate();
	}

	return v;
}

void QtPropertyDataIntrospection::ChildChanged(const QString &key, QtPropertyData *data)
{
	QtPropertyDataDavaVariant *dataVariant = (QtPropertyDataDavaVariant *) data;
	if(childIndexes.contains(dataVariant))
	{
		info->Member(childIndexes[dataVariant])->SetValue(object, dataVariant->GetVariantValue());
	}
}

void QtPropertyDataIntrospection::ChildNeedUpdate()
{
	for(int i = 0; i < info->MembersCount(); ++i)
	{
		QtPropertyDataDavaVariant *childData = childIndexes.key(i);
		DAVA::VariantType childCurValue = info->Member(i)->Value(object);

		if(childCurValue != childData->GetVariantValue())
		{
			childData->SetVariantValue(childCurValue);
		}
	}
}