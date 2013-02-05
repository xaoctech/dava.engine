#include "DAVAEngine.h"
#include "QtPropertyEditor/QtProperyData/QtPropertyDataIntrospection.h"
#include "QtPropertyEditor/QtProperyData/QtPropertyDataDavaVariant.h"
#include "QtPropertyEditor/QtProperyData/QtPropertyDataIntoCollection.h"

QtPropertyDataIntrospection::QtPropertyDataIntrospection(void *_object, const DAVA::IntrospectionInfo *_info)
	: object(_object)
	, info(_info)
{
	//int j = 0;
	while(NULL != _info && object)
	{
		for(DAVA::int32 i = 0; i < info->MembersCount(); ++i)
		{
			const DAVA::IntrospectionMember *member = _info->Member(i);
			if(member)
			{
                AddMember(member);
			}
		}

		//j += _info->MembersCount();
		_info = _info->BaseInfo();
	}

	SetFlags(FLAG_IS_DISABLED);
}

QtPropertyDataIntrospection::~QtPropertyDataIntrospection()
{ }

void QtPropertyDataIntrospection::AddMember(const DAVA::IntrospectionMember *member)
{
	void *memberObject = member->Data(object);
	const DAVA::MetaInfo *memberMetaInfo = member->Type();
	const DAVA::IntrospectionInfo *memberIntrospection = memberMetaInfo->GetIntrospection(memberObject);

    if(NULL != memberObject && NULL != memberIntrospection)
    {
		QtPropertyDataIntrospection *childData = new QtPropertyDataIntrospection(memberObject, memberIntrospection);
		ChildAdd(member->Name(), childData);
    }
    else
    {
        if(memberMetaInfo->IsPointer())
        {
			QString s;
            QtPropertyData* childData = new QtPropertyData(s.sprintf("[%p] Pointer", memberObject));
            childData->SetFlags(childData->GetFlags() | FLAG_IS_DISABLED);
            ChildAdd(member->Name(), childData);
        }
        else
        {
            if(member->Collection())
            {
                QtPropertyDataIntroCollection *childCollection = new QtPropertyDataIntroCollection(memberObject, member->Collection());
                ChildAdd(member->Name(), childCollection);
            }
            else
            {
                QtPropertyDataDavaVariant *childData = new QtPropertyDataDavaVariant(member->Value(object));
                if(member->Flags() & DAVA::INTROSPECTION_EDITOR_READONLY)
                {
                    childData->SetFlags(childData->GetFlags() | FLAG_IS_NOT_EDITABLE);
                }
                
                ChildAdd(member->Name(), childData);
                childVariantMembers.insert(childData, member);
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

	if(childVariantMembers.contains(dataVariant))
	{
		const DAVA::IntrospectionMember *member = childVariantMembers[dataVariant];
		member->SetValue(object, dataVariant->GetVariantValue());
	}
}

void QtPropertyDataIntrospection::ChildNeedUpdate()
{
	QMapIterator<QtPropertyDataDavaVariant*, const DAVA::IntrospectionMember *> i = QMapIterator<QtPropertyDataDavaVariant*, const DAVA::IntrospectionMember *>(childVariantMembers);

	while(i.hasNext())
	{
		i.next();

		QtPropertyDataDavaVariant *childData = i.key();
		DAVA::VariantType childCurValue = i.value()->Value(object);

		if(childCurValue != childData->GetVariantValue())
		{
			childData->SetVariantValue(childCurValue);
		}

	}
}
