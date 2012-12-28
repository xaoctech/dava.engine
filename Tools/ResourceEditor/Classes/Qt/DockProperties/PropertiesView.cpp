#include "DockProperties/PropertiesView.h"
#include "Scene/SceneDataManager.h"
#include <QTimer>

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

	// update timer
	QTimer::singleShot(200, this, SLOT(PropertiesUpdate()));
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

	PropertiesAllClear();

	if(NULL != node)
	{
		const DAVA::IntrospectionInfo *typeInfo = node->GetTypeInfo();
		while(NULL != typeInfo)
		{
			QtProperty *gr = managerGroup->addProperty(typeInfo->Name());
			managerGroup;

			for(int i = 0; i < typeInfo->MembersCount(); ++i)
			{
				QtProperty *pr = PropertyCreate(typeInfo->Member(i));
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

void PropertiesView::PropertiesAllClear()
{
	clear();

	managerGroup->clear();
	managerBool->clear();
	managerEnum->clear();
	managerInt->clear();
	managerString->clear();
}

void PropertiesView::PropertiesAllUpdate()
{
	QMapIterator<QtProperty *, const DAVA::IntrospectionMember *> i(allProperties);

	while(i.hasNext())
	{
		i.next();
		PropertySet(i.key(), i.value()->Value(curNode));
	}
}

QtProperty* PropertiesView::PropertyCreate(const DAVA::IntrospectionMember *member)
{
	QtProperty *pr = NULL;

	if(NULL != member)
	{
		if(member->Type() == DAVA::MetaInfo::Instance<int>())
		{
			pr = managerInt->addProperty(member->Name());
		}
		else if(member->Type() == DAVA::MetaInfo::Instance<bool>())
		{
			pr = managerBool->addProperty(member->Name());
		}
		else if(member->Type() == DAVA::MetaInfo::Instance<DAVA::String>())
		{
			pr = managerString->addProperty(member->Name());
		}
		else if(member->Type() == DAVA::MetaInfo::Instance<float32>())
		{
			pr = managerDouble->addProperty(member->Name());
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
		}
		else if(member->Type() == DAVA::MetaInfo::Instance<DAVA::Vector3>())
		{
			pr = managerVector3->addProperty(member->Name());
		}
		else if(member->Type() == DAVA::MetaInfo::Instance<DAVA::Vector4>())
		{
			pr = managerVector4->addProperty(member->Name());
		}
		else
		{
			DVASSERT("Unsupported type");
		}
	}

	if(NULL != pr)
	{
		allProperties[pr] = member;
		PropertySet(pr, member->Value(curNode));
	}

	return pr;
}

void PropertiesView::PropertySet(QtProperty *pr, const DAVA::VariantType &value)
{
	const DAVA::IntrospectionMember *member = NULL;

	if(allProperties.contains(pr))
	{
		member = allProperties[pr];
	}

	if(NULL != member)
	{
		if(member->Type() == DAVA::MetaInfo::Instance<int>())
		{
			managerInt->setValue(pr, member->Value(curNode).AsInt32());
		}
		else if(member->Type() == DAVA::MetaInfo::Instance<bool>())
		{
			managerBool->setValue(pr, member->Value(curNode).AsBool());
		}
		else if(member->Type() == DAVA::MetaInfo::Instance<DAVA::String>())
		{
			managerString->setValue(pr, member->Value(curNode).AsString().c_str());
		}
		else if(member->Type() == DAVA::MetaInfo::Instance<float32>())
		{
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
			managerVector2->setValue(pr, member->Value(curNode).AsVector2());
		}
		else if(member->Type() == DAVA::MetaInfo::Instance<DAVA::Vector3>())
		{
			managerVector3->setValue(pr, member->Value(curNode).AsVector3());
		}
		else if(member->Type() == DAVA::MetaInfo::Instance<DAVA::Vector4>())
		{
			managerVector4->setValue(pr, member->Value(curNode).AsVector4());
		}
		else
		{
			DVASSERT("Unsupported type");
		}
	}
}

void PropertiesView::PropertiesUpdate()
{
	if(NULL != curNode)
	{
		PropertiesAllUpdate();
	}

	QTimer::singleShot(200, this, SLOT(PropertiesUpdate()));
}

void PropertiesView::PropertyChangedInt(QtProperty *property)
{
	PropertySetToNode(property, managerInt->value(property));
}

void PropertiesView::PropertyChangedBool(QtProperty *property)
{
	PropertySetToNode(property, managerBool->value(property));
}

void PropertiesView::PropertyChangedEnum(QtProperty *property)
{
	PropertySetToNode(property, managerEnum->value(property));
}

void PropertiesView::PropertyChangedString(QtProperty *property)
{
	PropertySetToNode(property, managerString->value(property).toStdString());
}

void PropertiesView::PropertyChangedDouble(QtProperty *property)
{
	PropertySetToNode(property, (DAVA::float32) managerDouble->value(property));
}

void PropertiesView::PropertyChangedVector4(QtProperty *property)
{
	PropertySetToNode(property, managerVector4->value(property));
}

void PropertiesView::PropertyChangedVector3(QtProperty *property)
{
	PropertySetToNode(property, managerVector3->value(property));
}

template <typename T>
void PropertiesView::PropertySetToNode(QtProperty *property, const T &value)
{
	if(allProperties.contains(property) && NULL != curNode)
	{
		const DAVA::IntrospectionMember *member = allProperties[property];
		DAVA::VariantType varianlValue = DAVA::VariantType::LoadData(&value, DAVA::MetaInfo::Instance<T>());

		member->SetValue(curNode, varianlValue);
	}
}
