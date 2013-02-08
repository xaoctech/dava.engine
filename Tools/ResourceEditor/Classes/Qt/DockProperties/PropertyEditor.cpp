#include "DAVAEngine.h"
#include "Scene/SceneDataManager.h"
#include "Entity/Component.h"

#include "DockProperties/PropertyEditor.h"
#include "QtPropertyEditor/QtPropertyItem.h"
#include "QtPropertyEditor/QtProperyData/QtPropertyDataIntrospection.h"
#include "QtPropertyEditor/QtProperyData/QtPropertyDataDavaVariant.h"

#include "PropertyEditorStateHelper.h"

PropertyEditor::PropertyEditor(QWidget *parent /* = 0 */)
	: QtPropertyEditor(parent)
	, curNode(NULL)
	, treeStateHelper(this, this->curModel)
{
	// global scene manager signals
	QObject::connect(SceneDataManager::Instance(), SIGNAL(SceneActivated(SceneData *)), this, SLOT(sceneActivated(SceneData *)));
	QObject::connect(SceneDataManager::Instance(), SIGNAL(SceneChanged(SceneData *)), this, SLOT(sceneChanged(SceneData *)));
	QObject::connect(SceneDataManager::Instance(), SIGNAL(SceneReleased(SceneData *)), this, SLOT(sceneReleased(SceneData *)));
	QObject::connect(SceneDataManager::Instance(), SIGNAL(SceneNodeSelected(SceneData *, DAVA::SceneNode *)), this, SLOT(sceneNodeSelected(SceneData *, DAVA::SceneNode *)));
}

PropertyEditor::~PropertyEditor()
{
	SafeRelease(curNode);
}

void PropertyEditor::SetNode(DAVA::SceneNode *node)
{
#if 0
	TestIntro *intro = new TestIntro();
	AppendIntrospectionInfo(intro, intro->GetTypeInfo());
	SaveIntospection(intro, intro->GetTypeInfo());

	Test(node);
	return;
#endif

	// Store the current Property Editor Tree state before switching to the new node.
	// Do not clear the current states map - we are using one storage to share opened
	// Property Editor nodes between the different Scene Nodes.
	treeStateHelper.SaveTreeViewState(false);

	SafeRelease(curNode);
	curNode = SafeRetain(node);

	RemovePropertyAll();
	if(NULL != curNode)
	{
        AppendIntrospectionInfo(curNode, curNode->GetTypeInfo());

		for(int32 i = 0; i < Component::COMPONENT_COUNT; ++i)
        {
            Component *component = curNode->GetComponent(i);
            if(component)
            {
                AppendIntrospectionInfo(component, component->GetTypeInfo());
            }
        }
	}

	// Restore back the tree view state from the shared storage.
	if (!treeStateHelper.IsTreeStateStorageEmpty())
	{
		treeStateHelper.RestoreTreeViewState();
	}
	else
	{
		// Expand the root elements as default value.
		expandToDepth(0);
	}
}

void PropertyEditor::AppendIntrospectionInfo(void *object, const DAVA::IntrospectionInfo *info)
{
	if(NULL != info)
	{
		bool hasMembers = false;
		const IntrospectionInfo *currentInfo = info;

		// check if there are any memebers
		while (NULL != currentInfo)
		{
			if(currentInfo->MembersCount() > 0)
			{
				hasMembers = true;
				break;
			}
			currentInfo = currentInfo->BaseInfo();
		}

        //if(hasMembers)
        {
            QPair<QtPropertyItem*, QtPropertyItem*> prop = AppendProperty(currentInfo->Name(), new QtPropertyDataIntrospection(object, currentInfo));
            
            prop.first->setBackground(QBrush(QColor(Qt::lightGray)));
            prop.second->setBackground(QBrush(QColor(Qt::lightGray)));
        }
    }
}

void PropertyEditor::sceneChanged(SceneData *sceneData)
{
	if(NULL != sceneData)
	{
		SetNode(sceneData->GetSelectedNode());
	}
}

void PropertyEditor::sceneActivated(SceneData *sceneData)
{
	if(NULL != sceneData)
	{
		SetNode(sceneData->GetSelectedNode());
	}
}

void PropertyEditor::sceneReleased(SceneData *sceneData)
{ }

void PropertyEditor::sceneNodeSelected(SceneData *sceneData, DAVA::SceneNode *node)
{
	SetNode(node);
}


void PropertyEditor::Test(DAVA::BaseObject *object)
{
	SceneData *sd = SceneDataManager::Instance()->SceneGetActive();
	DAVA::SceneNode *rootNode = SceneDataManager::Instance()->SceneGetRootNode(sd);

	if(NULL != rootNode)
	{
		AppendIntrospectionInfo(rootNode, rootNode->GetTypeInfo());
		SaveIntospection(rootNode, rootNode->GetTypeInfo());
	}
}

void PropertyEditor::SaveIntospection(void *object, const DAVA::IntrospectionInfo *info)
{
	char tmp[128];
	DAVA::KeyedArchive *retArch = new DAVA::KeyedArchive();
	DAVA::KeyedArchive *tmpArch;

	DAVA::Map<void *, const DAVA::IntrospectionInfo *> objectsList;
	DAVA::Map<void *, const DAVA::IntrospectionInfo *>::iterator i;

	SearchIntrospection(object, info, &objectsList);

	tmpArch = SerializeIntrospection(object, info);
	sprintf(tmp, "%ld %s", tmpArch->GetArchiveId(), info->Name());
	retArch->SetArchive(tmp, tmpArch);
	tmpArch->Release();

	for (i = objectsList.begin(); i != objectsList.end(); ++i)
	{
		tmpArch = SerializeIntrospection(i->first, i->second);
		sprintf(tmp, "%ld %s", tmpArch->GetArchiveId(), i->second->Name());
		retArch->SetArchive(tmp, tmpArch);
		tmpArch->Release();
	}

	DumpKeyedArchive(retArch);
	printf("\n\n");
	retArch->Release();
}

DAVA::KeyedArchive* PropertyEditor::SerializeIntrospection(void *object, const DAVA::IntrospectionInfo *info)
{
	DAVA::KeyedArchive* ret = new DAVA::KeyedArchive();

	ret->SetArchiveId((DAVA::uint64) object);
	while(NULL != info && NULL != object)
	{
		for (int i = 0; i < info->MembersCount(); ++i)
		{
			const DAVA::IntrospectionMember *member = info->Member(i);
			if(!member->Type()->IsPointer())
			{
				if(member->Type()->GetIntrospection())
				{
					ret->SetArchive(member->Name(), SerializeIntrospection(member->Data(object), member->Type()->GetIntrospection()));
				}
				else if(NULL != member->Collection())
				{
					ret->SetArchive(member->Name(), SerializeCollection(member->Data(object), member->Collection()));
				}
				else
				{
                    VariantType value = member->Value(object);
					ret->SetVariant(member->Name(), &value);
				}
			}
			else
			{
				DAVA::uint64 ptr = (DAVA::uint64) member->Data(object);
				ret->SetUInt64(member->Name(), ptr);
			}
		}

		info = info->BaseInfo();
	}

	return ret;
}

DAVA::KeyedArchive* PropertyEditor::SerializeCollection(void *object, const DAVA::IntrospectionCollectionBase *collection)
{
	DAVA::KeyedArchive* ret = new DAVA::KeyedArchive();

	ret->SetArchiveId((DAVA::uint64) object);
	if(NULL != collection && NULL != object)
	{
		char tmpName[10];
		int j = 0;
		DAVA::IntrospectionCollectionBase::Iterator i = collection->Begin(object);

		while(NULL != i)
		{
			sprintf(tmpName, "%d", j++);

			if(!collection->ItemType()->IsPointer())
			{
				// collection with introspection object
				if(NULL != collection->ItemType()->GetIntrospection())
				{
					ret->SetArchive(tmpName, SerializeIntrospection(collection->ItemData(i), collection->ItemType()->GetIntrospection()));
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

void PropertyEditor::SearchIntrospection(void *object, const DAVA::IntrospectionInfo *info, DAVA::Map<void *, const DAVA::IntrospectionInfo *> *result)
{
	if(NULL != info && NULL != object && NULL != result)
	{
		(*result)[object] = info;

		while(NULL != info && NULL != object)
		{
			for (int i = 0; i < info->MembersCount(); ++i)
			{
				const DAVA::IntrospectionMember *member = info->Member(i);
				if(NULL != member->Type()->GetIntrospection() &&
					member->Type()->IsPointer())
				{
					SearchIntrospection(member->Data(object), member->Type()->GetIntrospection(), result);
				}
				else if(NULL != member->Collection())
				{
					const IntrospectionCollectionBase* collection = member->Collection();
					void* collectionObject = member->Data(object);

					if(NULL != collection->ItemType()->GetIntrospection() &&
						collection->ItemType()->IsPointer())
					{
						DAVA::IntrospectionCollectionBase::Iterator ci = collection->Begin(collectionObject);
						while(NULL != ci)
						{
							SearchIntrospection(collection->ItemData(ci), collection->ItemType()->GetIntrospection(), result);
							ci = collection->Next(ci);
						}
					}
				}
			}

			info = info->BaseInfo();
		}
	}
}

void PropertyEditor::DumpKeyedArchive(DAVA::KeyedArchive *archive, int level)
{
	for (int j = 0; j < level; j++) { printf("  "); }
	printf("| ");
	printf("Archive %lu\n", archive->GetArchiveId());

	Map<String, VariantType*> map = archive->GetArchieveData();
	Map<String, VariantType*>::iterator i = map.begin();
	for(i; i != map.end(); ++i)
	{
		bool need_line_end = true;

		for (int j = 0; j < (level + 1); j++) { printf("  "); }
		printf("| ");

		printf("%s", i->first.c_str());
		for (int j = 0; j < (35 - (int) strlen(i->first.c_str()) - level * 2); j++) { printf(" ", i->first.c_str()); }
		printf(" : ");

		switch(i->second->GetType())
		{
		case VariantType::TYPE_KEYED_ARCHIVE:
			printf("is KeyedArchive\n");
			DumpKeyedArchive(i->second->AsKeyedArchive(), level + 2);
			need_line_end = false;
			break;
		case VariantType::TYPE_STRING:
			printf("%s", i->second->AsString().c_str());
			break;	
		case VariantType::TYPE_WIDE_STRING:
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
		case VariantType::TYPE_INT64:
			printf("%ld", i->second->AsInt64());
			break;
		case VariantType::TYPE_UINT64:
			printf("%lu", i->second->AsUInt64());
			break;
		default:
			const MetaInfo* varMeta = i->second->Meta();
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
	printf("| ---\n");
}
