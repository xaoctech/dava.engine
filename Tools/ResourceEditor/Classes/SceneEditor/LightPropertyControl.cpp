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
	propertyList->SetComboPropertyIndex("Type", light->GetType());

    propertyList->AddFloatProperty("r");
	propertyList->SetFloatPropertyValue("r", light->GetColor().r);

    propertyList->AddFloatProperty("g");
	propertyList->SetFloatPropertyValue("g", light->GetColor().g);

    propertyList->AddFloatProperty("b"); 
	propertyList->SetFloatPropertyValue("b", light->GetColor().b);

	propertyList->AddSection("Static light", GetHeaderState("Static light", true));

	propertyList->AddBoolProperty("Enable");
	propertyList->SetBoolPropertyValue("Enable", light->GetCustomProperties()->GetBool("editor.staticlight.enable", true));

	propertyList->AddBoolProperty("Cast shadows");
	propertyList->SetBoolPropertyValue("Cast shadows", light->GetCustomProperties()->GetBool("editor.staticlight.castshadows", true));

	propertyList->AddFloatProperty("Intensity");
	propertyList->SetFloatPropertyValue("Intensity", light->GetCustomProperties()->GetFloat("editor.intensity", 1.f));

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
		1.f);
	light->SetColor(color);

	int32 type = propertyList->GetComboPropertyIndex("Type");
	light->SetType((LightNode::eType)type);

	bool enable = propertyList->GetBoolPropertyValue("Enable");
	light->GetCustomProperties()->SetBool("editor.staticlight.enable", enable);

	bool castShadows = propertyList->GetBoolPropertyValue("Cast shadows");
	light->GetCustomProperties()->SetBool("editor.staticlight.castshadows", castShadows);

	float32 intensity = propertyList->GetFloatPropertyValue("Intensity");
	light->GetCustomProperties()->SetFloat("editor.intensity", intensity);
}
