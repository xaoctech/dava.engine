#include "LightPropertyControl.h"


LightPropertyControl::LightPropertyControl(const Rect & rect, bool createNodeProperties)
:	NodesPropertyControl(rect, createNodeProperties)
{
	types.push_back("Directional");
	types.push_back("Spot");
	types.push_back("Point");
	types.push_back("Sky");
}

LightPropertyControl::~LightPropertyControl()
{

}

void LightPropertyControl::ReadFrom(SceneNode * sceneNode)
{
	NodesPropertyControl::ReadFrom(sceneNode);

	LightNode * light = dynamic_cast<LightNode *>(sceneNode);
	DVASSERT(light);

    propertyList->AddSection("Light", GetHeaderState("Light", true));
        
    propertyList->AddComboProperty("Type", types);
    propertyList->AddFloatProperty("r");
    propertyList->AddFloatProperty("g");
    propertyList->AddFloatProperty("b"); 
    propertyList->AddFloatProperty("a"); 
	//propertyList->AddFloatProperty("Intensity");

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
