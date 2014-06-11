/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/



#include "LibraryController.h"
#include "HierarchyTreeControlNode.h"
#include "MetadataFactory.h"
#include "ScreenWrapper.h"
#include "DefaultScreen.h"
#include "IconHelper.h"

using namespace DAVA;

static const QString AGGREGATOR_CONTROL_NAME = "UIAggregatorControl";

LibraryController::LibraryController()
{
	widget = NULL;
}

LibraryController::~LibraryController()
{
	for (CONTROLS::iterator iter = controls.begin(); iter != controls.end(); ++iter)
	{
		HierarchyTreeNode* node = iter->first;
		SafeDelete(node);
	}
}

void LibraryController::Init(LibraryWidget* widget)
{
	this->widget = widget;
	AddStandardControls();
}

void LibraryController::AddStandardControls()
{
    AddControl("UIControl", new UIControl());
	AddControl("UIButton", new UIButton());
	AddControl("UIStaticText", new UIStaticText());
	AddControl("UITextField", new UITextField());
	AddControl("UISlider", new UISlider());
	AddControl("UIList", new UIList());
	AddControl("UIScrollBar", new UIScrollBar());
	AddControl("UIScrollView", new UIScrollView());
	AddControl("UISpinner", new UISpinner());
	AddControl("UISwitch", new UISwitch());
   	AddControl("UIParticles", new UIParticles());
    AddControl("UIWebView", new UIWebView());
    AddControl("UIMovieView", new UIMovieView());
    AddControl("UIJoypad", new UIJoypad());
    AddControl("UI3DView", new UI3DView());
}

void LibraryController::AddControl(HierarchyTreeAggregatorNode* node)
{
	HierarchyTreePlatformNode* nodePlatform = node->GetPlatform();
    if (!nodePlatform)
		return;
    
	HierarchyTreePlatformNode* activePlatform = HierarchyTreeController::Instance()->GetActivePlatform();
	bool needFullUpdate = activePlatform && (node->GetPlatform()->GetId() != activePlatform->GetId());
    if (!needFullUpdate)
	{
		const QString& name = node->GetName();
		DVASSERT(controls.find(node) == controls.end());
		controls[node] = widget->AddControl(name, IconHelper::GetIconPathForClassName(AGGREGATOR_CONTROL_NAME));
	}

    HierarchyTreeNode::HIERARCHYTREENODEID platformId = nodePlatform->GetId();
    AGGREGATORS aggregatorsList = GetPlatformAggregators(platformId);
    aggregatorsList.push_back(node);
    platforms[platformId] = aggregatorsList;
    UpdateLibrary(needFullUpdate);
    
}

void LibraryController::RemoveControl(HierarchyTreeAggregatorNode* node)
{
	HierarchyTreePlatformNode* nodePlatform = node->GetPlatform();
    if (!nodePlatform)
    	return;
    
    HierarchyTreeNode::HIERARCHYTREENODEID platformId = nodePlatform->GetId();
    AGGREGATORS aggregatorsList = GetPlatformAggregators(platformId);
    if (!aggregatorsList.empty())
    {
   	 	aggregatorsList.remove(node);
    	platforms[platformId] = aggregatorsList;
	}
	CONTROLS::iterator iter = controls.find(node);
	if (iter == controls.end())
		return;
	
	widget->RemoveControl(iter->second);
	controls.erase(node);
}

void LibraryController::UpdateControl(HierarchyTreeAggregatorNode* node)
{
	CONTROLS::iterator iter = controls.find(node);
	if (iter == controls.end())
		return;

	widget->UpdateControl(iter->second, node->GetName());
}

void LibraryController::AddControl(const QString& name, UIControl* control)
{
	QString iconPath = IconHelper::GetIconPathForUIControl(control);
	controls[new HierarchyTreeControlNode(NULL, control, name)] = widget->AddControl(name, iconPath);
}

HierarchyTreeControlNode* LibraryController::CreateNewControl(HierarchyTreeNode* parentNode, const QString& strType, const QString& name, const Vector2& position)
{
	String type = strType.toStdString();

	HierarchyTreeControlNode* controlNode = NULL;
	UIControl* control = NULL;
	CONTROLS::iterator iter;
	
	for (iter = controls.begin(); iter != controls.end(); ++iter)
	{
		HierarchyTreeNode* node = iter->first;
		if (strType == node->GetName())
			break;
	}
	
	if (iter == controls.end() ||
		dynamic_cast<HierarchyTreeControlNode*>(iter->first))
	{
		//create standart control
		BaseObject* object = ObjectFactory::Instance()->New<BaseObject>(type);
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
		HierarchyTreeAggregatorNode* aggregator = dynamic_cast<HierarchyTreeAggregatorNode*>(iter->first);
		if (aggregator)
		{
			controlNode = aggregator->CreateChild(parentNode, name);
			control = controlNode->GetUIObject();
		}
	}
	
	parentNode->AddTreeNode(controlNode);
	
	// In case the control has subcontrols - they should be added to the control node too.
	if (control && !control->GetSubcontrols().empty())
	{
		List<UIControl*> subControls = control->GetSubcontrols();
		for (List<UIControl*>::iterator iter = subControls.begin(); iter != subControls.end(); iter ++)
		{
			UIControl* subControl = (*iter);
			if (!subControl)
			{
				continue;
			}

			HierarchyTreeControlNode* subControlNode =
				new HierarchyTreeControlNode(controlNode, subControl,
											 QString::fromStdString(subControl->GetName()));
			controlNode->AddTreeNode(subControlNode);
		}
	}

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

void LibraryController::UpdateLibrary(bool needFullUpdate)
{
	HierarchyTreePlatformNode* activePlatform = HierarchyTreeController::Instance()->GetActivePlatform();
	HierarchyTreeScreenNode* activeScreen = HierarchyTreeController::Instance()->GetActiveScreen();

    if (needFullUpdate)
    {
    	controls.clear();
    	widget->ClearAllItems();
        AddStandardControls();

    	if (activePlatform)
   		{
     	    AGGREGATORS aggregatorsList = GetPlatformAggregators(activePlatform->GetId());
       		for (AGGREGATORS::iterator iter = aggregatorsList.begin(); iter != aggregatorsList.end(); ++iter)
        	{
        		HierarchyTreeAggregatorNode *aggregatorNode = (*iter);
        		controls[aggregatorNode] = widget->AddControl(aggregatorNode->GetName(), IconHelper::GetIconPathForClassName(AGGREGATOR_CONTROL_NAME));
        	}
   		}
	}
    else
    {
    	for (CONTROLS::iterator iter = controls.begin(); iter != controls.end(); ++iter)
		{
			HierarchyTreeAggregatorNode* node = dynamic_cast<HierarchyTreeAggregatorNode*>(iter->first);
			if (!node)
				continue;
		
			bool visible = activePlatform && (node->GetPlatform()->GetId() == activePlatform->GetId());
			if (dynamic_cast<HierarchyTreeAggregatorNode*>(activeScreen))
			{
				visible = false;
			}
			widget->SetItemEnabled(iter->second, visible);
		}
    }
	// DF-1488 - Reset treeview widget state, including selection
	widget->ResetSelection();
}

LibraryController::AGGREGATORS LibraryController::GetPlatformAggregators(HierarchyTreeNode::HIERARCHYTREENODEID platformId)
{
    PLATFORMS::iterator iter = platforms.find(platformId);
    AGGREGATORS aggregatorsList;
    
    if (iter != platforms.end())
    {
    	aggregatorsList = iter->second;
    }
    
	return aggregatorsList;
}