#include "DAVAEngine.h"
#include "Scene/SceneDataManager.h"

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

	RemovePropertyAll();
	if(NULL != curNode)
	{
		if(!sss)
		{
			curNode->GetCustomProperties()->SetBool("111", true);
			curNode->GetCustomProperties()->SetArchive("subArchive", DAVA::Core::Instance()->GetOptions());
			sss = true;
		}

		const DAVA::IntrospectionInfo *info = curNode->GetTypeInfo();
		while(NULL != info)
		{
			QPair<QtPropertyItem*, QtPropertyItem*> prop = AppendProperty(info->Name(), new QtPropertyDataIntrospection(node, info));

			prop.first->setBackground(QBrush(QColor(Qt::lightGray)));
			prop.second->setBackground(QBrush(QColor(Qt::lightGray)));

			info = info->BaseInfo();
		}
	}

	expandToDepth(0);
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
