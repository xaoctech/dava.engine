#include "DAVAEngine.h"
#include "QtPropertyEditor/QtProperyData/QtPropertyDataIntrospection.h"
#include "QtPropertyEditor/QtProperyData/QtPropertyDataDavaVariant.h"
#include "QtPropertyEditor/QtProperyData/QtPropertyDataIntoCollection.h"

QtPropertyDataIntrospection::QtPropertyDataIntrospection(void *_object, const DAVA::IntrospectionInfo *_info)
	: object(_object)
	, info(_info)
{
	if(NULL != info && object)
	{
		for(DAVA::int32 i = 0; i < info->MembersCount(); ++i)
		{
			const DAVA::IntrospectionMember *member = info->Member(i);
			if(member && (member->Flags() & DAVA::INTROSPECTION_EDITOR))
			{
                AddMember(member, i);
			}
		}
	}

	SetFlags(FLAG_IS_DISABLED);
}

QtPropertyDataIntrospection::~QtPropertyDataIntrospection()
{ }

void QtPropertyDataIntrospection::AddMember(const DAVA::IntrospectionMember *member, const DAVA::int32 &index)
{
    const DAVA::MetaInfo *memberMetaInfo = member->Type();

    if(memberMetaInfo->GetIntrospection())
    {
        AddMemberIntrospection(member, index);
    }
    else
    {
        if(memberMetaInfo->IsPointer())
        {
            QtPropertyData* childData = new QtPropertyData(QString("##intro_poiner##"));
            childData->SetFlags(childData->GetFlags() | FLAG_IS_NOT_EDITABLE);
            ChildAdd(member->Name(), childData);
            childVariantIndexes.insert(NULL, index);
        }
        else
        {
            if(member->Collection())
            {
                QtPropertyDataIntroCollection *childCollection = new QtPropertyDataIntroCollection(member->Pointer(object), member->Collection());
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

void QtPropertyDataIntrospection::AddMemberIntrospection(const DAVA::IntrospectionMember *member, const DAVA::int32 &index)
{
    const DAVA::MetaInfo *memberMetaInfo = member->Type();
    const DAVA::IntrospectionInfo *memberInfo = memberMetaInfo->GetIntrospection();
    void *memberObject = NULL;
    if(memberMetaInfo->IsPointer())
    {
        memberObject = *((void **)member->Pointer(object));
    }
    else
    {
        memberObject = member->Pointer(object);
    }
    
    QtPropertyDataIntrospection *parent = this;
    DAVA::int32 currentIndex = index;
    
    while (memberInfo)
    {
        QtPropertyDataIntrospection *childData = new QtPropertyDataIntrospection(memberObject, memberInfo);
        
        parent->ChildAdd(memberInfo->Name(), childData);
        parent->childVariantIndexes.insert(NULL, currentIndex);
        
        memberInfo = memberInfo->BaseInfo();
        currentIndex = 0;
        parent = childData;
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
