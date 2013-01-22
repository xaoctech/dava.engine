#include "QtPropertyEditor/QtProperyData/QtPropertyDataIntoCollection.h"
#include "QtPropertyEditor/QtProperyData/QtPropertyDataIntrospection.h"
#include "QtPropertyEditor/QtProperyData/QtPropertyDataDavaVariant.h"

QtPropertyDataIntroCollection::QtPropertyDataIntroCollection(void *_object, const DAVA::IntrospectionCollectionBase *_collection)
	: object(_object)
	, collection(_collection)
{
	if(NULL != collection && collection->Size(object) > 0)
	{
		int index = 0;
		DAVA::MetaInfo *valueType = collection->ValueType();
		DAVA::IntrospectionCollectionBase::Iterator i = collection->Begin(object);
		while(NULL != i)
		{
			if(NULL != valueType->GetIntrospection())
			{
				QtPropertyData *childData;
				if(!valueType->IsPointer())
				{
					childData = new QtPropertyDataIntrospection(collection->ItemPointer(i), valueType->GetIntrospection());
				}
				else
				{
					childData = new QtPropertyDataIntrospection(*((void **)collection->ItemPointer(i)), valueType->GetIntrospection());
				}

				ChildAdd(QString::number(index), childData);
			}
			else
			{
				if(!valueType->IsPointer())
				{
					QtPropertyDataDavaVariant *childData = new QtPropertyDataDavaVariant(DAVA::VariantType::LoadData(collection->ItemPointer(i), valueType));
					ChildAdd(QString::number(index), childData);
				}
				else
				{
					QtPropertyData* childData = new QtPropertyData(QString("##collect_poiner##"));
					childData->SetFlags(FLAG_IS_NOT_EDITABLE);
					ChildAdd(QString::number(index), childData);
				}
			}

			index++;
			i = collection->Next(i);
		}
	}

	SetFlags(FLAG_IS_DISABLED);
}

QtPropertyDataIntroCollection::~QtPropertyDataIntroCollection()
{ }

QVariant QtPropertyDataIntroCollection::GetValueInternal()
{
	ChildNeedUpdate();
	return QVariant(collection->CollectionType()->GetTypeName());
}
