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
	managerVector2 = new QtVector2PropertyManager(oneForAllParent);
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
	setFactoryForManager(managerVector2->subDoublePropertyManager(), editorDouble);
	setFactoryForManager(managerVector3->subDoublePropertyManager(), editorDouble);
	setFactoryForManager(managerVector4->subDoublePropertyManager(), editorDouble);

	// Property changed signals
	QObject::connect(managerInt, SIGNAL(propertyChanged(QtProperty *)), this, SLOT(PropertyChangedInt(QtProperty *)));
	QObject::connect(managerBool, SIGNAL(propertyChanged(QtProperty *)), this, SLOT(PropertyChangedBool(QtProperty *)));
	QObject::connect(managerEnum, SIGNAL(propertyChanged(QtProperty *)), this, SLOT(PropertyChangedEnum(QtProperty *)));
	QObject::connect(managerString, SIGNAL(propertyChanged(QtProperty *)), this, SLOT(PropertyChangedString(QtProperty *)));
	QObject::connect(managerDouble, SIGNAL(propertyChanged(QtProperty *)), this, SLOT(PropertyChangedDouble(QtProperty *)));
	QObject::connect(managerVector4, SIGNAL(propertyChanged(QtProperty *)), this, SLOT(PropertyChangedVector4(QtProperty *)));
	QObject::connect(managerVector3, SIGNAL(propertyChanged(QtProperty *)), this, SLOT(PropertyChangedVector3(QtProperty *)));

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
			managerGroup;

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
			pr = managerInt->addProperty(member->Name());
			managerInt->setValue(pr, member->Value(curNode).AsInt32());
		}
		else if(member->Type() == DAVA::MetaInfo::Instance<bool>())
		{
			pr = managerBool->addProperty(member->Name());
			managerBool->setValue(pr, member->Value(curNode).AsBool());
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
			DVASSERT("Templory unsupported type");
		}
		else if(member->Type() == DAVA::MetaInfo::Instance<DAVA::Matrix3>())
		{
			DVASSERT("Templory unsupported type");
		}
		else if(member->Type() == DAVA::MetaInfo::Instance<DAVA::Matrix4>())
		{
			DVASSERT("Templory unsupported type");
		}
		else if(member->Type() == DAVA::MetaInfo::Instance<DAVA::Vector2>())
		{
			pr = managerVector2->addProperty(member->Name());
			managerVector2->setValue(pr, member->Value(curNode).AsVector2());
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
			DVASSERT("Unsupported type");
		}
	}

	if(NULL != pr)
	{
		allProperties[pr] = member;
	}

	return pr;
}

void PropertiesView::PropertyChangedInt(QtProperty *property)
{
	PropertyChange(property, managerInt->value(property));
}

void PropertiesView::PropertyChangedBool(QtProperty *property)
{
	PropertyChange(property, managerBool->value(property));
}

void PropertiesView::PropertyChangedEnum(QtProperty *property)
{
	PropertyChange(property, managerEnum->value(property));
}

void PropertiesView::PropertyChangedString(QtProperty *property)
{
	PropertyChange(property, managerString->value(property).toStdString());
}

void PropertiesView::PropertyChangedDouble(QtProperty *property)
{
	PropertyChange(property, (DAVA::float32) managerDouble->value(property));
}

void PropertiesView::PropertyChangedVector4(QtProperty *property)
{
	PropertyChange(property, managerVector4->value(property));
}

void PropertiesView::PropertyChangedVector3(QtProperty *property)
{
	PropertyChange(property, managerVector3->value(property));
}

template <typename T>
void PropertiesView::PropertyChange(QtProperty *property, T value)
{
	if(allProperties.contains(property) && NULL != curNode)
	{
		const DAVA::IntrospectionMember *member = allProperties[property];
		DAVA::VariantType varianlValue = DAVA::VariantType::LoadData(&value, DAVA::MetaInfo::Instance<T>());

		member->SetValue(curNode, varianlValue);
	}
}
