#include "SwitchNodePropertyControl.h"

SwitchNodePropertyControl::SwitchNodePropertyControl(const Rect & rect, bool createNodeProperties)
:	NodesPropertyControl(rect, createNodeProperties)
{

}

void SwitchNodePropertyControl::ReadFrom(Entity * sceneNode)
{
	NodesPropertyControl::ReadFrom(sceneNode);

	propertyList->AddSection("Switch node", GetHeaderState("Switch node", true));

	Vector<String> switchIndeces;
	int32 childrenCount = sceneNode->GetChildrenCount();
	for(int32 i = 0; i < childrenCount; i++) 
	{
		switchIndeces.push_back(Format("%d", i));
	}
	if(switchIndeces.empty())
	{
		switchIndeces.push_back("0");
	}

    
    SwitchComponent *switchComponent = static_cast<SwitchComponent *>(sceneNode->GetComponent(Component::SWITCH_COMPONENT));
    if(switchComponent)
    {
        propertyList->AddComboProperty("Switch index", switchIndeces);
        propertyList->SetComboPropertyIndex("Switch index", switchComponent->GetSwitchIndex());
    }
}

void SwitchNodePropertyControl::OnComboIndexChanged(PropertyList *forList, const String &forKey, int32 newItemIndex, const String &newItemKey)
{
    SwitchComponent *switchComponent = static_cast<SwitchComponent *>(currentSceneNode->GetComponent(Component::SWITCH_COMPONENT));
    if(switchComponent)
    {
        if("Switch index" == forKey)
        {
            switchComponent->SetSwitchIndex(newItemIndex);
        }
    }
    
	NodesPropertyControl::OnComboIndexChanged(forList, forKey, newItemIndex, newItemKey);
}
