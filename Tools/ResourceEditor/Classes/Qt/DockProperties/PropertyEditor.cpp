#include "DAVAEngine.h"
#include "Scene/SceneDataManager.h"
#include "Entity/Component.h"

#include "DockProperties/PropertyEditor.h"
#include "QtPropertyEditor/QtPropertyItem.h"
#include "QtPropertyEditor/QtProperyData/QtPropertyDataIntrospection.h"
#include "QtPropertyEditor/QtProperyData/QtPropertyDataDavaVariant.h"

PropertyEditor::PropertyEditor(QWidget *parent /* = 0 */)
	: QtPropertyEditor(parent)
	, curNode(NULL)
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
	static bool sss = false;

	SafeRelease(curNode);
	curNode = SafeRetain(node);

	//printf("%p, HasIntrospection: %d\n", node, (MetaInfo::Instance(node)->GetIntrospectionInfo() != NULL));
	printf("SceneNode isIntrospection: %d\n", HasIntrospection<DAVA::SceneNode>::result);
	//printf("%p, HasIntrospection: = %p\n", node, HasIntrospection<DAVA::SceneNode>::introspection());
	printf("SceneNode introspection: = %p\n", doSomething(node));

	//printf("%p, This HasIntrospection: %d\n", this, (MetaInfo::Instance(this)->GetIntrospectionInfo() != NULL));
	printf("PropertyEditor isIntrospection: %d\n", HasIntrospection<PropertyEditor>::result);
	//printf("%p, This HasIntrospection: %p\n", this, HasIntrospection<PropertyEditor>::introspection());
	printf("PropertyEditor introspection: = %p\n", doSomething(this));

	//printf("2) %p, This HasIntrospection: %d\n", this, (MetaInfo::Instance(this)->GetIntrospectionInfo() != NULL));

	RemovePropertyAll();
	if(NULL != curNode)
	{
		if(!sss)
		{
			curNode->GetCustomProperties()->SetBool("111", true);
			curNode->GetCustomProperties()->SetArchive("subArchive", DAVA::Core::Instance()->GetOptions());
			sss = true;
		}

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

	expandToDepth(0);
}

void PropertyEditor::AppendIntrospectionInfo(void *object, const DAVA::IntrospectionInfo *info)
{
    while(NULL != info)
    {
        //if(info->MembersCount())
        {
            QPair<QtPropertyItem*, QtPropertyItem*> prop = AppendProperty(info->Name(), new QtPropertyDataIntrospection(object, info));
            
            prop.first->setBackground(QBrush(QColor(Qt::lightGray)));
            prop.second->setBackground(QBrush(QColor(Qt::lightGray)));
        }
        
        info = info->BaseInfo();
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
