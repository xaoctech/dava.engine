//
//  LibraryController.cpp
//  UIEditor
//
//  Created by adebt on 3/11/13.
//
//

#include "LibraryController.h"
#include "HierarchyTreeControlNode.h"
#include "MetadataFactory.h"
#include "ScreenWrapper.h"
#include "DefaultScreen.h"

using namespace DAVA;

LibraryController::LibraryController()
{
	widget = NULL;
	aggregatorTemp = new UIAggregatorControl();
}

LibraryController::~LibraryController()
{
	SafeRelease(aggregatorTemp);
	for (CONTROLS::iterator iter = controls.begin(); iter != controls.end(); ++iter)
	{
		HierarchyTreeNode* node = iter->second;
		SafeDelete(node);
	}
}

void LibraryController::Init(LibraryWidget* widget)
{
	this->widget = widget;
	AddControl("UIControl", new UIControl());
	AddControl("UIButton", new UIButton());
	AddControl("UIStaticText", new UIStaticText());
	AddControl("UIList", new UIList());
	AddControl("UIScrollBar", new UIScrollBar());
}

void LibraryController::AddControl(HierarchyTreeAggregatorNode* node)
{
	const QString& name = node->GetName();
	DVASSERT(controls.find(name) == controls.end());
	controls[name] = node;
	widget->AddControl(name);
}

void LibraryController::RemoveControl(HierarchyTreeAggregatorNode* node)
{
	const QString& name = node->GetName();
	CONTROLS::iterator iter = controls.find(name);
	if (iter != controls.end())
		controls.erase(iter);
	widget->RemoveControl(name);
}

void LibraryController::UpdateControl(HierarchyTreeAggregatorNode* node)
{
	
}

void LibraryController::AddControl(const QString& name, UIControl* control)
{
	DVASSERT(controls.find(name) == controls.end());
	controls[name] = new HierarchyTreeControlNode(NULL, control, name);
	widget->AddControl(name);
}

bool LibraryController::IsNameAvailable(const QString& name) const
{
	return (controls.find(name) == controls.end());
}

HierarchyTreeControlNode* LibraryController::CreateNewControl(HierarchyTreeNode* parentNode, const QString& strType, const QString& name, const Vector2& position)
{
	String type = strType.toStdString();

	HierarchyTreeControlNode* controlNode = NULL;
	UIControl* control = NULL;
	CONTROLS::iterator iter = controls.find(strType);
	if (iter == controls.end() ||
		dynamic_cast<HierarchyTreeControlNode*>(iter->second))
	{
		//create standart control
		BaseObject* object = ObjectFactory::Instance()->New(type);
		control = dynamic_cast<UIControl*>(object);
		if (!control)
		{
			SafeRelease(object);
			return NULL;
		}
		 
		controlNode = new HierarchyTreeControlNode(parentNode, control, name);
	}
	else
	{
		//create aggregator
		HierarchyTreeAggregatorNode* aggregator = dynamic_cast<HierarchyTreeAggregatorNode*>(iter->second);
		if (aggregator)
		{
			controlNode = aggregator->CreateChild(parentNode, name);
			control = controlNode->GetUIObject();
		}
	}
	
	parentNode->AddTreeNode(controlNode);

	// Initialize a control through its metadata.
	BaseMetadata* newControlMetadata = MetadataFactory::Instance()->GetMetadataForUIControl(control);

	METADATAPARAMSVECT params;
	params.push_back(BaseMetadataParams(controlNode->GetId(), control));
	newControlMetadata->SetupParams(params);

	// Ready to do initialization!
	newControlMetadata->InitializeControl(name.toStdString(), position);

	SAFE_DELETE(newControlMetadata);

	SafeRelease(control);
	return controlNode;
}


