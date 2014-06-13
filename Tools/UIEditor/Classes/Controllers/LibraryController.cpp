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
/*	for (CONTROLS::iterator iter = controls.begin(); iter != controls.end(); ++iter)
	{
		HierarchyTreeNode* node = iter->first;
		SafeDelete(node);
	}*/
}

void LibraryController::Init(LibraryWidget* widget)
{
	this->widget = widget;
	AddStandardControls();
}

void LibraryController::AddStandardControls()
{
    AddControl("UIControl", new UIControl(), -10);
	AddControl("UIButton", new UIButton(), -11);
	AddControl("UIStaticText", new UIStaticText(), -12);
	AddControl("UITextField", new UITextField(), -13);
	AddControl("UISlider", new UISlider(), -14);
	AddControl("UIList", new UIList(), -15);
	AddControl("UIScrollBar", new UIScrollBar(), -16);
	AddControl("UIScrollView", new UIScrollView(), -17);
	AddControl("UISpinner", new UISpinner(), -18);
	AddControl("UISwitch", new UISwitch(), -19);
   	AddControl("UIParticles", new UIParticles(), -20);
    AddControl("UIWebView", new UIWebView(), -21);
    AddControl("UIMovieView", new UIMovieView(), -22);
    AddControl("UIJoypad", new UIJoypad(), -23);
    AddControl("UI3DView", new UI3DView(), -24);
}
/*
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
}*/

void LibraryController::AddControl(const QString& name, UIControl* control, int itemId)
{
	QString iconPath = IconHelper::GetIconPathForUIControl(control);
	controls[itemId] = widget->AddControl(name, iconPath);
}

HierarchyTreeControlNode* LibraryController::CreateNewControl(HierarchyTreeNode* parentNode, const QString& strType, const QString& name, const Vector2& position)
{
	String type = strType.toStdString();

	HierarchyTreeControlNode* controlNode = NULL;
	UIControl* control = NULL;
	CONTROLS::iterator iter;
	
	for (iter = controls.begin(); iter != controls.end(); ++iter)
	{
        HierarchyTreeNode::HIERARCHYTREENODEID nodeId = iter->first;
        HierarchyTreeNode* node = HierarchyTreeController::Instance()->GetTree().GetNode(nodeId);
        
        // Create aggregator
        if (node && strType == node->GetName())
        {
			HierarchyTreeAggregatorNode* aggregator = dynamic_cast<HierarchyTreeAggregatorNode*>(node);
			if (aggregator)
			{
				controlNode = aggregator->CreateChild(parentNode, name);
				control = controlNode->GetUIObject();
                break;
            }
        }
	}
	
	if (iter == controls.end() || !control)
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

void LibraryController::UpdateLibrary()
{
	HierarchyTreePlatformNode* activePlatform = HierarchyTreeController::Instance()->GetActivePlatform();
    
    if (!activePlatform)
    	return;
    
    List<HierarchyTreeAggregatorNode*> aggregatorsList = GetPlatformAggregators(activePlatform);
    
    CONTROLS aggregatorControls(controls);
    
    // Remove aggregators
    for (CONTROLS::iterator iter = aggregatorControls.begin(); iter != aggregatorControls.end(); ++iter)
    {
		HierarchyTreeNode::HIERARCHYTREENODEID nodeId = iter->first;
        HierarchyTreeAggregatorNode* node = dynamic_cast<HierarchyTreeAggregatorNode*>(HierarchyTreeController::Instance()->GetTree().GetNode(nodeId));
        
        if (!node)
        	continue;
        
        widget->RemoveControl(iter->second);
        controls.erase(nodeId);
    }
    
    
    // Add aggregators
    for (List<HierarchyTreeAggregatorNode*>::iterator it = aggregatorsList.begin(); it != aggregatorsList.end(); ++it)
    {
    	HierarchyTreeAggregatorNode* aggregatorNode = (*it);
        
        const QString& name = aggregatorNode->GetName();
        controls[aggregatorNode->GetId()] =  widget->AddControl(name, IconHelper::GetIconPathForClassName(AGGREGATOR_CONTROL_NAME));
	}
    
    /*bool visible = activePlatform && (node->GetPlatform()->GetId() == activePlatform->GetId());
	if (dynamic_cast<HierarchyTreeAggregatorNode*>(activeScreen))
	{
		visible = false;
	}*/
    
    //widget->SetItemEnabled(iter->second, visible);
    
    // Remove aggregator items
    
	//HierarchyTreeScreenNode* activeScreen = HierarchyTreeController::Instance()->GetActiveScreen();
    
    
    //widget->ClearAllItems();
    
    
/*
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
    }*/
	// DF-1488 - Reset treeview widget state, including selection
	widget->ResetSelection();
}

void LibraryController::RemoveAggregatorItems()
{
	for (CONTROLS::iterator iter = controls.begin(); iter != controls.end(); ++iter)
    {
    	HierarchyTreeNode::HIERARCHYTREENODEID nodeId = iter->first;
        HierarchyTreeNode* node = HierarchyTreeController::Instance()->GetTree().GetNode(nodeId);
        // Delete from library only existing nodes (all standard controls should remain)
		if (!node)
			continue;
        
        controls.erase(nodeId);
    }
}

List<HierarchyTreeAggregatorNode*> LibraryController::GetPlatformAggregators(HierarchyTreePlatformNode* activePlatform)
{
	List<HierarchyTreeAggregatorNode*> aggregatorsList;

	const HierarchyTreeNode::HIERARCHYTREENODESLIST& child = activePlatform->GetChildNodes();
	for (HierarchyTreeNode::HIERARCHYTREENODESLIST::const_iterator iter = child.begin();
		 iter != child.end();
		 ++iter)
	{
		HierarchyTreeAggregatorNode* aggregatorNode = dynamic_cast<HierarchyTreeAggregatorNode*>((*iter));
		if (!aggregatorNode)
			continue;
				
		aggregatorsList.push_back(aggregatorNode);
	}
    
	return aggregatorsList;
}