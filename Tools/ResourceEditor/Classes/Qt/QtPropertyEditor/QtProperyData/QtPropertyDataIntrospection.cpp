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
			if(member)
			{
				const DAVA::MetaInfo *memberMetaInfo = member->Type();
				printf("adding %s, type %s\n", member->Name(), memberMetaInfo->GetTypeName());

				if(memberMetaInfo->GetIntrospection())
				{
					QtPropertyData *childData = NULL;
					if(memberMetaInfo->IsPointer())
					{
						childData = new QtPropertyDataIntrospection(*((void **)member->Pointer(object)), memberMetaInfo->GetIntrospection());
					}
					else
					{
						childData = new QtPropertyDataIntrospection(member->Pointer(object), memberMetaInfo->GetIntrospection());
					}

					ChildAdd(member->Name(), childData);
					childVariantIndexes.insert(NULL, i);
				}
				else //if(member->Flags() & DAVA::INTROSPECTION_EDITOR)
				{
                    if(memberMetaInfo->IsPointer())
                    {
                        QtPropertyData* childData = new QtPropertyData(QString("##intro_poiner##"));
						childData->SetFlags(childData->GetFlags() | FLAG_IS_NOT_EDITABLE);
						ChildAdd(member->Name(), childData);
						childVariantIndexes.insert(NULL, i);
                    }
                    else
                    {
                        if(member->Collection())
                        {
                            QtPropertyDataIntroCollection *childCollection = new QtPropertyDataIntroCollection(member->Pointer(object), member->Collection());
							ChildAdd(member->Name(), childCollection);
							childVariantIndexes.insert(NULL, i);
                        }
                        else
                        {
                            QtPropertyDataDavaVariant *childData = new QtPropertyDataDavaVariant(member->Value(object));
							if(member->Flags() & DAVA::INTROSPECTION_EDITOR_READONLY)
							{
                                childData->SetFlags(childData->GetFlags() | FLAG_IS_NOT_EDITABLE);
							}
                            
							ChildAdd(member->Name(), childData);
							childVariantIndexes.insert(childData, i);
                        }
                    }
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
