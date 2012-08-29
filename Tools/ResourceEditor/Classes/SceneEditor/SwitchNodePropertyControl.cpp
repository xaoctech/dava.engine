#include "SceneEditor/SwitchNodePropertyControl.h"

SwitchNodePropertyControl::SwitchNodePropertyControl(const Rect & rect, bool createNodeProperties)
:	NodesPropertyControl(rect, createNodeProperties)
{

}

void SwitchNodePropertyControl::ReadFrom(SceneNode * sceneNode)
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

	SwitchNode * switchNode = dynamic_cast<SwitchNode*>(sceneNode);
	propertyList->AddComboProperty("Switch index", switchIndeces);
	propertyList->SetComboPropertyIndex("Switch index", switchNode->GetSwitchIndex());
}

void SwitchNodePropertyControl::OnComboIndexChanged(PropertyList *forList, const String &forKey, int32 newItemIndex, const String &newItemKey)
{
	SwitchNode * switchNode = dynamic_cast<SwitchNode*>(currentSceneNode);

	if("Switch index" == forKey)
	{
		switchNode->SetSwitchIndex(newItemIndex);
	}
}
