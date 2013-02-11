#include "DAVAEngine.h"
#include "Base/Introspection.h"
#include "FileSystem/VariantType.h"
#include "Entity/Component.h"
#include "SceneFileV3.h"

/*
#if 0

TestIntro *intro = new TestIntro();
Test(intro);
AppendIntrospectionInfo(intro, intro->GetTypeInfo());

//Test(node);
//return;
#endif
*/

/*
REGISTER_CLASS(TestIntro)
REGISTER_CLASS(TestBase)
REGISTER_CLASS(TestBaseChild)

void SceneFileV3::Test(DAVA::BaseObject *object)
{
	if(NULL != object)
	{	
		DAVA::KeyedArchive *arch = SaveIntospection(object, object->GetTypeInfo());

		DumpKeyedArchive(arch);

		void *loaded = LoadIntrospection(arch);
		//AppendIntrospectionInfo(loaded, MetaInfo::Instance<DAVA::BaseObject>()->GetIntrospection(object));

		arch->Release();
	}
}
}*/

DAVA::KeyedArchive* SceneFileV3::SaveIntospection(void *object, const DAVA::IntrospectionInfo *info)
{
	char tmp[128];
	DAVA::KeyedArchive *retArch = new DAVA::KeyedArchive();
	DAVA::KeyedArchive *tmpArch;

	DAVA::Map<void *, const DAVA::IntrospectionInfo *> objectsList;
	DAVA::Map<void *, const DAVA::IntrospectionInfo *>::iterator i;

	SearchIntrospection(object, info, &objectsList);

	//tmpArch = SerializeIntrospection(object, info);
	//retArch->SetArchive("##head", tmpArch);
	//tmpArch->Release();
	//printf("saved: %llu\n", (DAVA::uint64) object);

	for (i = objectsList.begin(); i != objectsList.end(); ++i)
	{
		tmpArch = SerializeIntrospection(i->first, i->second->Type()->GetIntrospection(i->first));
		if(i->first == object)
		{
			retArch->SetArchive("##head", tmpArch);
		}
		else
		{
			sprintf(tmp, "%llu", tmpArch->GetUInt64("##id"));
			retArch->SetArchive(tmp, tmpArch);
		}
		tmpArch->Release();
		printf("saved: %p as %llu\n", i->first, (DAVA::uint64) i->first);
	}

	return retArch;
}

DAVA::KeyedArchive* SceneFileV3::SerializeIntrospection(void *object, const DAVA::IntrospectionInfo *info)
{
	DAVA::KeyedArchive* ret = new DAVA::KeyedArchive();
	DAVA::uint64 id = (DAVA::uint64) object;

	ret->SetString("##name", info->Name());
	ret->SetUInt64("##id", id);

	const DAVA::IntrospectionInfo *baseInfo = info;
	while (NULL != baseInfo->BaseInfo())
	{
		baseInfo = baseInfo->BaseInfo();
	}
	if(baseInfo == DAVA::MetaInfo::Instance<DAVA::Component>()->GetIntrospection() && NULL != object)
	{
		DAVA::Component *component = (DAVA::Component *) object;
		ret->SetUInt32("##type", component->GetType());
	}

	while(NULL != info && NULL != object)
	{
		for (int i = 0; i < info->MembersCount(); ++i)
		{
			const DAVA::IntrospectionMember *member = info->Member(i);
			void *memberData = member->Data(object);

			if(!member->Type()->IsPointer())
			{
				if(member->Type()->GetIntrospection())
				{
					ret->SetArchive(member->Name(), SerializeIntrospection(memberData, member->Type()->GetIntrospection(memberData)));
				}
				else if(NULL != member->Collection())
				{
					ret->SetArchive(member->Name(), SerializeCollection(memberData, member->Collection()));
				}
				else
				{
					ret->SetVariant(member->Name(), &(member->Value(object)));
				}
			}
			else
			{
				if(member->Type() == DAVA::MetaInfo::Instance<DAVA::KeyedArchive *>())
				{
					ret->SetArchive(member->Name(), member->Value(object).AsKeyedArchive());
				}
				else
				{
					DAVA::uint64 ptr = (DAVA::uint64) memberData;
					ret->SetUInt64(member->Name(), ptr);
				}
			}
		}

		info = info->BaseInfo();
	}

	return ret;
}

DAVA::KeyedArchive* SceneFileV3::SerializeCollection(void *object, const DAVA::IntrospectionCollection *collection)
{
	DAVA::KeyedArchive* ret = new DAVA::KeyedArchive();

	if(NULL != collection && NULL != object)
	{
		char tmpName[10];
		int j = 0;
		DAVA::IntrospectionCollection::Iterator i = collection->Begin(object);

		while(NULL != i)
		{
			void *itemData = collection->ItemData(i);

			sprintf(tmpName, "%d", j++);

			// if collection doesn't contain pointers
			if(!collection->ItemType()->IsPointer())
			{
				// collection with introspection object
				if(NULL != collection->ItemType()->GetIntrospection())
				{
					if(NULL != itemData)
					{
						DAVA::MetaInfo *itemType = collection->ItemType();
						const DAVA::IntrospectionInfo *itemIntrospection = itemType->GetIntrospection();

						if(NULL != itemData)
						{
							itemIntrospection = itemType->GetIntrospection(itemData);
						}

						ret->SetArchive(tmpName, SerializeIntrospection(collection->ItemData(i), itemIntrospection));
					}
					else
					{
						DAVA::uint64 d = (DAVA::uint64) itemData;
						ret->SetUInt64(tmpName, d);
					}
				}
				// collection with any other type
				else
				{
					// ret->SetVariant(tmpName, collection->It)
					// TODO:
					// ...
					// 
					// 

					printf("Don't know how to serialize\n");
				}
			}
			// pointers in collection
			else
			{
				DAVA::uint64 ptr = (DAVA::uint64) collection->ItemData(i);
				ret->SetUInt64(tmpName, ptr);
			}

			i = collection->Next(i);
		}
	}

	return ret;
}

void SceneFileV3::SearchIntrospection(void *object, const DAVA::IntrospectionInfo *info, DAVA::Map<void *, const DAVA::IntrospectionInfo *> *result, bool skipFirst)
{
	if(NULL != info && NULL != object && NULL != result)
	{
		if(result->find(object) != result->end())
		{
			return;
		}

		if(!skipFirst)
		{
			(*result)[object] = info;
			printf("found: %p %s\n", object, info->Type()->GetIntrospection(object)->Name());
		}

		while(NULL != info && NULL != object)
		{
			for (int i = 0; i < info->MembersCount(); ++i)
			{
				const DAVA::IntrospectionMember *member = info->Member(i);
				if(NULL != member->Type()->GetIntrospection() &&
					member->Type()->IsPointer())
				{
					printf("member name: %s\n", member->Name());
					SearchIntrospection(member->Data(object), member->Type()->GetIntrospection(member->Data(object)), result);
				}
				else if(NULL != member->Collection())
				{
					const DAVA::IntrospectionCollection* collection = member->Collection();
					void* collectionObject = member->Data(object);

					if(NULL != collection->ItemType()->GetIntrospection() &&
						collection->ItemType()->IsPointer())
					{
						DAVA::IntrospectionCollection::Iterator ci = collection->Begin(collectionObject);
						while(NULL != ci)
						{
							SearchIntrospection(collection->ItemData(ci), collection->ItemType()->GetIntrospection(collection->ItemData(ci)), result);
							ci = collection->Next(ci);
						}
					}
				}
			}

			info = info->BaseInfo();
		}
	}
}

void SceneFileV3::DumpKeyedArchive(DAVA::KeyedArchive *archive, int level)
{
	/*
	if(level > 0)
	{
		for (int j = 0; j < level - 1; j++) { printf("  "); }
		printf("| ");
	}

	printf("Archive %llu\n", archive->GetUInt64("##id"));
	*/

	DAVA::Map<DAVA::String, DAVA::VariantType*> map = archive->GetArchieveData();
	DAVA::Map<DAVA::String, DAVA::VariantType*>::iterator i = map.begin();
	for(i; i != map.end(); ++i)
	{
		bool need_line_end = true;

		for (int j = 0; j < level; j++) { printf("  "); }
		printf("| ");

		printf("%s", i->first.c_str());
		for (int j = 0; j < (35 - (int) strlen(i->first.c_str()) - level * 2); j++) { printf(" ", i->first.c_str()); }
		printf(" : ");

		switch(i->second->GetType())
		{
		case DAVA::VariantType::TYPE_KEYED_ARCHIVE:
			printf("is KeyedArchive\n");
			DumpKeyedArchive(i->second->AsKeyedArchive(), level + 1);
			need_line_end = false;
			break;
		case DAVA::VariantType::TYPE_STRING:
			printf("%s", i->second->AsString().c_str());
			break;	
		case DAVA::VariantType::TYPE_WIDE_STRING:
			printf("%s", i->second->AsWideString().c_str());
			break;
			/*
		case VariantType::TYPE_BOOLEAN:
			break;
		case VariantType::TYPE_INT32:
			break;	
		case VariantType::TYPE_UINT32:
			break;	
		case VariantType::TYPE_FLOAT:
			break;	
			*/
		case DAVA::VariantType::TYPE_INT64:
			printf("%lld", i->second->AsInt64());
			break;
		case DAVA::VariantType::TYPE_UINT64:
			printf("%llu", i->second->AsUInt64());
			break;
		case DAVA::VariantType::TYPE_INT32:
			printf("%d", i->second->AsInt32());
			break;
		case DAVA::VariantType::TYPE_UINT32:
			printf("%u", i->second->AsUInt32());
			break;
		default:
			const DAVA::MetaInfo* varMeta = i->second->Meta();
			if(NULL != varMeta)
			{
				printf("%s", varMeta->GetTypeName());
			}
			else
			{
				printf("unknown");
			}
		}

		if(need_line_end) printf("\n");
	}

	for (int j = 0; j < level; j++) { printf("  "); }
	printf("| -----\n");
}


void* SceneFileV3::LoadIntrospection(DAVA::KeyedArchive *archive)
{
	void *first = NULL;

	if(NULL != archive)
	{
		DAVA::Map<DAVA::String, DAVA::VariantType*> map = archive->GetArchieveData();
		DAVA::Map<DAVA::String, DAVA::VariantType*>::iterator i = map.begin();
		DAVA::Map<void **, DAVA::uint64> structure;
		DAVA::Map<DAVA::uint64, void *> createdObjects;

		for(i; i != map.end(); ++i)
		{
			DAVA::KeyedArchive *objectArch = i->second->AsKeyedArchive();

			if(NULL != objectArch)
			{
				DAVA::uint64 id = objectArch->GetUInt64("##id", 0);
				DAVA::String className = objectArch->GetString("##name");

				if(0 != id)
				{
					void *object = NULL;
					const DAVA::IntrospectionInfo* info = NULL;

					if(objectArch->IsKeyExists("##type"))
					{
						object = DAVA::Component::CreateByType(objectArch->GetUInt32("##type"));
						info = DAVA::MetaInfo::Instance<DAVA::Component>()->GetIntrospection(object);
					}
					else
					{
						object = DAVA::ObjectFactory::Instance()->New(className);
						info = DAVA::MetaInfo::Instance<DAVA::BaseObject>()->GetIntrospection(object);
					}

					if(NULL == object)
					{
						DVASSERT(0 && "Don't know how to create");
					}
				
					printf("created: %llu, %s, %p\n", id, info->Name(), object);
					if(NULL != object)
					{
						createdObjects[id] = object;
						DeserializeIntrospection(object, info, i->second->AsKeyedArchive(), &structure);

						if(i->first == "##head")
						{
							DumpKeyedArchive(objectArch);
							first = object;
						}
					}
				}
			}
		}

		printf("restoring pointers:\n");
		DAVA::Map<void **, DAVA::uint64>::iterator j = structure.begin();
		for (; j != structure.end(); ++j)
		{
			if(createdObjects.find(j->second) != createdObjects.end())
			{
				*j->first = createdObjects[j->second];
				printf(" %llu -> %p\n", j->second, createdObjects[j->second]);
			}
			else
			{
				printf(" %llu -> failed\n", j->second);
			}
		}

	}

	return first;
}

void SceneFileV3::DeserializeIntrospection(void *object, const DAVA::IntrospectionInfo *info, DAVA::KeyedArchive *archive, DAVA::Map<void **, DAVA::uint64> *structure)
{
	if(NULL != object && NULL != archive && NULL != info)
	{
		while(NULL != info)
		{
			for (int i = 0; i < info->MembersCount(); ++i)
			{
				const DAVA::IntrospectionMember *member = info->Member(i);
				void *memberData = member->Data(object);

				if(archive->IsKeyExists(member->Name()))
				{
					if(!member->Type()->IsPointer())
					{
						if(member->Type()->GetIntrospection())
						{
							DeserializeIntrospection(memberData, member->Type()->GetIntrospection(memberData), archive->GetArchive(member->Name()), structure);
						}
						else if(NULL != member->Collection())
						{
							DeserializeCollection(memberData, member->Collection(), archive->GetArchive(member->Name()), structure);
						}
						else
						{
							DAVA::VariantType v = *(archive->GetVariant(member->Name()));
							member->SetValue(object, v);
						}
					}
					else
					{
						if(member->Type() == DAVA::MetaInfo::Instance<DAVA::KeyedArchive *>())
						{
							DAVA::VariantType v = *(archive->GetVariant(member->Name()));
							member->SetValue(object, v);
						}
						else
						{
							void **ptr = (void **) member->Pointer(object);
							DAVA::uint64 id = archive->GetUInt64(member->Name(), 0);	
							printf("link: %llu\n", id);
							if(0 != id)
							{
								(*structure)[ptr] = id;
							}
						}
					}
				}
			}

			info = info->BaseInfo();
		}
	}
}

void SceneFileV3::DeserializeCollection(void *object, const DAVA::IntrospectionCollection *collection, DAVA::KeyedArchive *archive, DAVA::Map<void **, DAVA::uint64> *structure)
{
	if(NULL != object && NULL != collection && NULL != archive)
	{
		int maxIndex = -1;
		int index = 0;
		DAVA::Map<int, DAVA::VariantType*> variantIndexes;

		DAVA::Map<DAVA::String, DAVA::VariantType*> map = archive->GetArchieveData();
		DAVA::Map<DAVA::String, DAVA::VariantType*>::iterator i = map.begin();

		for( ;i != map.end(); ++i)
		{
			if(sscanf(i->first.c_str(), "%d", &index) > 0)
			{
				variantIndexes[index] = i->second;
				if(index > maxIndex)
				{
					maxIndex = index;
				}
			}
		}

		if(maxIndex >= collection->Size(object) && maxIndex >= 0)
		{
			collection->Resize(object, maxIndex + 1);
		}

		index = 0;
		DAVA::IntrospectionCollection::Iterator j = collection->Begin(object);
		while(NULL != j)
		{
			if(!collection->ItemType()->IsPointer())
			{
				if(NULL != collection->ItemType()->GetIntrospection())
				{
					void *itemData = collection->ItemData(j);
					DAVA::MetaInfo *itemType = collection->ItemType();
					const DAVA::IntrospectionInfo *itemIntrospection = itemType->GetIntrospection();

					DeserializeIntrospection(itemData, itemIntrospection, variantIndexes[index]->AsKeyedArchive(), structure);
				}
				else
				{
					// TODO:
					// set base value
					// ...
					// collection->ItemValueSet(i, variantIndexes[index]->);
				}
			}
			else
			{
				//*ptr = NULL;
				//* 
				// remember pointer on pointer for future load
				void **ptr = (void **) collection->ItemPointer(j);
				DAVA::uint64 id = variantIndexes[index]->AsUInt64();
				printf("link: %llu\n", id);
				if(0 != id)
				{
					(*structure)[ptr] = id;
				}

			}
			
			index++;
			j = collection->Next(j);
		}
	}
}
