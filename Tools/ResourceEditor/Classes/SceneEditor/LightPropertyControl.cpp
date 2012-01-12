#include "LightPropertyControl.h"


LightPropertyControl::LightPropertyControl(const Rect & rect, bool createNodeProperties)
:	NodesPropertyControl(rect, createNodeProperties)
{

}

LightPropertyControl::~LightPropertyControl()
{

}

void LightPropertyControl::ReadFrom(SceneNode * sceneNode)
{
	NodesPropertyControl::ReadFrom(sceneNode);

	LightNode * light = dynamic_cast<LightNode *>(sceneNode);
	DVASSERT(light);

    propertyList->AddSection("Light", headerStates->GetBool("Light", true));
        
    propertyList->AddComboProperty("Type", types);
    propertyList->AddFloatProperty("r", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("g", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("b", PropertyList::PROPERTY_IS_EDITABLE); 
    propertyList->AddFloatProperty("a", PropertyList::PROPERTY_IS_EDITABLE); 

    propertyList->SetComboPropertyIndex("Type", light->GetType());
    propertyList->SetFloatPropertyValue("r", light->GetColor().r);
    propertyList->SetFloatPropertyValue("g", light->GetColor().g);
    propertyList->SetFloatPropertyValue("b", light->GetColor().b);
    propertyList->SetFloatPropertyValue("a", light->GetColor().a);
}

void LightPropertyControl::WriteTo(SceneNode * sceneNode)
{
	NodesPropertyControl::WriteTo(sceneNode);

	LightNode *light = dynamic_cast<LightNode *>(sceneNode);
	DVASSERT(light);

	Color color(
		propertyList->GetFloatPropertyValue("r"),
		propertyList->GetFloatPropertyValue("g"),
		propertyList->GetFloatPropertyValue("b"),
		propertyList->GetFloatPropertyValue("a"));

	int32 type = propertyList->GetComboPropertyIndex("Type");

	light->SetColor(color);
	light->SetType((LightNode::eType)type);
}
