#include "DAVAEngine.h"
#include "QtPropertyEditor/QtProperyData/QtPropertyDataIntrospection.h"
#include "QtPropertyEditor/QtProperyData/QtPropertyDataDavaVariant.h"

//#include "Base/IntrospectionFlags.h"

QtPropertyDataIntrospection::QtPropertyDataIntrospection(void *_object, const DAVA::IntrospectionInfo *_info)
	: object(_object)
	, info(_info)
{
	if(NULL != info)
	{
		for(int i = 0; i < info->MembersCount(); ++i)
		{
			const DAVA::IntrospectionMember *member = info->Member(i);
			if(NULL != member)
			{
				const DAVA::MetaInfo *memberMetaInfo = member->Type();

				// check if member has introspection
				if(NULL != memberMetaInfo->GetIntrospection())
				{
					QtPropertyData *childData = new QtPropertyDataIntrospection(member->Pointer(object), memberMetaInfo->GetIntrospection());
					ChildAdd(info->Member(i)->Name(), childData);
					childVariantIndexes.insert(NULL, i);
				}
				else
				{
					QtPropertyDataDavaVariant *childData = new QtPropertyDataDavaVariant(info->Member(i)->Value(object));
                    
                    if(info->Member(i)->Flags() & DAVA::INTROSPECTION_FLAG_EDITOR_READONLY)
                    {
                        int flags = childData->GetFlags();
                        childData->SetFlags(flags | FLAG_IS_NOT_EDITABLE);
                    }
                    
					ChildAdd(info->Member(i)->Name(), childData);
					childVariantIndexes.insert(childData, i);
				}
			}
		}
	}

	SetFlags(FLAG_IS_DISABLED);
}

QtPropertyDataIntrospection::~QtPropertyDataIntrospection()
{ }

QVariant QtPropertyDataIntrospection::GetValueInternal()
{
	ChildNeedUpdate();
	return QVariant();
}

void QtPropertyDataIntrospection::ChildChanged(const QString &key, QtPropertyData *data)
{
	QtPropertyDataDavaVariant *dataVariant = (QtPropertyDataDavaVariant *) data;
	if(childVariantIndexes.contains(dataVariant))
	{
		info->Member(childVariantIndexes[dataVariant])->SetValue(object, dataVariant->GetVariantValue());
	}
}

void QtPropertyDataIntrospection::ChildNeedUpdate()
{
	for(int i = 0; i < info->MembersCount(); ++i)
	{
		QtPropertyDataDavaVariant *childData = childVariantIndexes.key(i);
		if(NULL != childData)
		{
			DAVA::VariantType childCurValue = info->Member(i)->Value(object);

			if(childCurValue != childData->GetVariantValue())
			{
				childData->SetVariantValue(childCurValue);
			}
		}
	}
}
