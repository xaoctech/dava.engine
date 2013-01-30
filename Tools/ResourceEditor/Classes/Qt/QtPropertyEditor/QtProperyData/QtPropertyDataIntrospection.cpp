#include "DAVAEngine.h"
#include "QtPropertyEditor/QtProperyData/QtPropertyDataIntrospection.h"
#include "QtPropertyEditor/QtProperyData/QtPropertyDataDavaVariant.h"
#include "QtPropertyEditor/QtProperyData/QtPropertyDataIntoCollection.h"

QtPropertyDataIntrospection::QtPropertyDataIntrospection(void *_object, const DAVA::IntrospectionInfo *_info)
	: object(_object)
	, info(_info)
{
	while(NULL != _info && object)
	{
		for(DAVA::int32 i = 0; i < info->MembersCount(); ++i)
		{
			const DAVA::IntrospectionMember *member = _info->Member(i);
			if(member)
			{
                AddMember(member, i);
			}
		}

		_info = _info->BaseInfo();
	}

	SetFlags(FLAG_IS_DISABLED);
}

QtPropertyDataIntrospection::~QtPropertyDataIntrospection()
{ }

void QtPropertyDataIntrospection::AddMember(const DAVA::IntrospectionMember *member, const DAVA::int32 &index)
{
	void *memberObject = member->Data(object);
	const DAVA::MetaInfo *memberMetaInfo = member->Type();
	const DAVA::IntrospectionInfo *memberIntrospection = memberMetaInfo->GetIntrospection(memberObject);

    if(NULL != memberObject && NULL != memberIntrospection)
    {
		QtPropertyDataIntrospection *childData = new QtPropertyDataIntrospection(memberObject, memberIntrospection);
		ChildAdd(member->Name(), childData);
		childVariantIndexes.insert(NULL, index);
    }
    else
    {
        if(memberMetaInfo->IsPointer())
        {
			QString s;
            QtPropertyData* childData = new QtPropertyData(s.sprintf("[%p] Pointer", memberObject));
            childData->SetFlags(childData->GetFlags() | FLAG_IS_DISABLED);
            ChildAdd(member->Name(), childData);
            childVariantIndexes.insert(NULL, index);
        }
        else
        {
            if(member->Collection())
            {
                QtPropertyDataIntroCollection *childCollection = new QtPropertyDataIntroCollection(memberObject, member->Collection());
                ChildAdd(member->Name(), childCollection);
                childVariantIndexes.insert(NULL, index);
            }
            else
            {
                QtPropertyDataDavaVariant *childData = new QtPropertyDataDavaVariant(member->Value(object));
                if(member->Flags() & DAVA::INTROSPECTION_EDITOR_READONLY)
                {
                    childData->SetFlags(childData->GetFlags() | FLAG_IS_NOT_EDITABLE);
                }
                
                ChildAdd(member->Name(), childData);
                childVariantIndexes.insert(childData, index);
            }
        }
    }
}

QVariant QtPropertyDataIntrospection::GetValueInternal()
{
	ChildNeedUpdate();
	return QVariant(info->Name());
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
