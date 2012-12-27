#include "DockProperties/PropertiesView.h"
#include "Scene/SceneDataManager.h"

PropertiesView::PropertiesView(QWidget *parent /* = 0 */)
	: QtTreePropertyBrowser(parent)
	, curNode(NULL)
{
	// parent widget
	oneForAllParent = new QWidget();

	// property managers
	managerGroup = new QtGroupPropertyManager(oneForAllParent);
	managerInt = new QtIntPropertyManager(oneForAllParent);
	managerBool = new QtBoolPropertyManager(oneForAllParent);
	managerEnum = new QtEnumPropertyManager(oneForAllParent);
	managerString = new QtStringPropertyManager(oneForAllParent);
	managerDouble = new QtDoublePropertyManager(oneForAllParent);
	managerVector3 = new QtVector3PropertyManager(oneForAllParent);
	managerVector4 = new QtVector4PropertyManager(oneForAllParent);

	// property editors
	editorInt = new QtSpinBoxFactory(oneForAllParent);
	editorBool = new QtCheckBoxFactory(oneForAllParent);
	editorString = new QtLineEditFactory(oneForAllParent);
	editorEnum = new QtEnumEditorFactory(oneForAllParent);
	editorDouble = new QtDoubleSpinBoxFactory(oneForAllParent);

	// setup property managers with appropriate property editors
	setFactoryForManager(managerInt, editorInt);
	setFactoryForManager(managerBool, editorBool);
	setFactoryForManager(managerEnum, editorEnum);
	setFactoryForManager(managerString, editorString);
	setFactoryForManager(managerDouble, editorDouble);

	// global scene manager signals
	QObject::connect(SceneDataManager::Instance(), SIGNAL(SceneActivated(SceneData *)), this, SLOT(sceneActivated(SceneData *)));
	QObject::connect(SceneDataManager::Instance(), SIGNAL(SceneChanged(SceneData *)), this, SLOT(sceneChanged(SceneData *)));
	QObject::connect(SceneDataManager::Instance(), SIGNAL(SceneReleased(SceneData *)), this, SLOT(sceneReleased(SceneData *)));
	QObject::connect(SceneDataManager::Instance(), SIGNAL(SceneNodeSelected(SceneData *, DAVA::SceneNode *)), this, SLOT(sceneNodeSelected(SceneData *, DAVA::SceneNode *)));
}

PropertiesView::~PropertiesView()
{
	SafeRelease(curNode);

	delete oneForAllParent;
}

void PropertiesView::SetNode(DAVA::SceneNode *node)
{
	SafeRelease(curNode);
	curNode = SafeRetain(node);

	ClearAllProperties();

	if(NULL != node)
	{
		const DAVA::IntrospectionInfo *typeInfo = node->GetTypeInfo();
		while(NULL != typeInfo)
		{
			QtProperty *gr = managerGroup->addProperty(typeInfo->Name());

			for(int i = 0; i < typeInfo->MembersCount(); ++i)
			{
				QtProperty *pr = CreateProperty(typeInfo->Member(i));
				if(NULL != pr)
				{
					gr->addSubProperty(pr);
				}
			}

			addProperty(gr);
			typeInfo = typeInfo->BaseInfo();
		}
	}
}

void PropertiesView::sceneChanged(SceneData *sceneData)
{
	if(NULL != sceneData)
	{
		SetNode(sceneData->GetSelectedNode());
	}
}

void PropertiesView::sceneActivated(SceneData *sceneData)
{
	if(NULL != sceneData)
	{
		SetNode(sceneData->GetSelectedNode());
	}
}

void PropertiesView::sceneReleased(SceneData *sceneData)
{ }

void PropertiesView::sceneNodeSelected(SceneData *sceneData, DAVA::SceneNode *node)
{
	SetNode(node);
}

void PropertiesView::ClearAllProperties()
{
	clear();

	managerGroup->clear();
	managerBool->clear();
	managerEnum->clear();
	managerInt->clear();
	managerString->clear();
}

QtProperty* PropertiesView::CreateProperty(const DAVA::IntrospectionMember *member)
{
	QtProperty *pr = NULL;

	if(NULL != member)
	{
		if(member->Type() == DAVA::MetaInfo::Instance<int>())
		{
			
		}
		else if(member->Type() == DAVA::MetaInfo::Instance<bool>())
		{

		}
		else if(member->Type() == DAVA::MetaInfo::Instance<DAVA::String>())
		{
			pr = managerString->addProperty(member->Name());
			managerString->setValue(pr, member->Value(curNode).AsString().c_str());
		}
		else if(member->Type() == DAVA::MetaInfo::Instance<float32>())
		{
			pr = managerDouble->addProperty(member->Name());
			managerDouble->setValue(pr, member->Value(curNode).AsFloat());
		}
		else if(member->Type() == DAVA::MetaInfo::Instance<DAVA::Matrix2>())
		{

		}
		else if(member->Type() == DAVA::MetaInfo::Instance<DAVA::Matrix3>())
		{

		}
		else if(member->Type() == DAVA::MetaInfo::Instance<DAVA::Matrix4>())
		{

		}
		else if(member->Type() == DAVA::MetaInfo::Instance<DAVA::Vector2>())
		{

		}
		else if(member->Type() == DAVA::MetaInfo::Instance<DAVA::Vector3>())
		{
			pr = managerVector3->addProperty(member->Name());
			managerVector3->setValue(pr, member->Value(curNode).AsVector3());
		}
		else if(member->Type() == DAVA::MetaInfo::Instance<DAVA::Vector4>())
		{
			pr = managerVector4->addProperty(member->Name());
			managerVector4->setValue(pr, member->Value(curNode).AsVector4());
		}
		else
		{
		}
	}

	if(NULL != pr)
	{
		allProperties[pr] = member;
	}

	return pr;
}