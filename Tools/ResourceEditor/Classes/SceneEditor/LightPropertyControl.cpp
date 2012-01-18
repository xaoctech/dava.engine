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

    propertyList->AddSection("property.lightnode.light", GetHeaderState("property.lightnode.light", true));
	propertyList->SetBoolPropertyValue("Used in static lighting", sceneNode->GetCustomProperties()->GetBool("editor.staticlight.used", false));
        
    propertyList->AddComboProperty("property.lightnode.type", types);
	propertyList->SetComboPropertyIndex("property.lightnode.type", light->GetType());

    propertyList->AddFloatProperty("property.lightnode.r");
	propertyList->SetFloatPropertyValue("property.lightnode.r", light->GetColor().r);

    propertyList->AddFloatProperty("property.lightnode.g");
	propertyList->SetFloatPropertyValue("property.lightnode.g", light->GetColor().g);

    propertyList->AddFloatProperty("property.lightnode.b"); 
	propertyList->SetFloatPropertyValue("property.lightnode.b", light->GetColor().b);

	propertyList->AddSection("property.lightnode.staticlight", GetHeaderState("property.lightnode.staticlight", true));

	propertyList->AddBoolProperty("Enable");
	propertyList->SetBoolPropertyValue("Enable", light->GetCustomProperties()->GetBool("editor.staticlight.enable", true));

	propertyList->AddBoolProperty("Cast shadows");
	propertyList->SetBoolPropertyValue("Cast shadows", light->GetCustomProperties()->GetBool("editor.staticlight.castshadows", true));

	propertyList->AddFloatProperty("Intensity");
	propertyList->SetFloatPropertyValue("Intensity", light->GetCustomProperties()->GetFloat("editor.intensity", 1.f));

	if(LightNode::TYPE_DIRECTIONAL == light->GetType())
	{
		propertyList->AddFloatProperty("Shadow angle");
		propertyList->SetFloatPropertyValue("Shadow angle", light->GetCustomProperties()->GetFloat("editor.shadowangle", 0.f));

		propertyList->AddIntProperty("Shadow samples");
		propertyList->SetIntPropertyValue("Shadow samples", light->GetCustomProperties()->GetInt32("editor.shadowsamples", 1));
	}
}

void LightPropertyControl::OnComboIndexChanged(
                                    PropertyList *forList, const String &forKey, int32 newItemIndex, const String &newItemKey)
{
    if("property.lightnode.type" == forKey)
    {
        LightNode *light = dynamic_cast<LightNode *>(currentNode);
        light->SetType((LightNode::eType)newItemIndex);
        
        if(LightNode::TYPE_DIRECTIONAL == light->GetType())
        {
            light->GetCustomProperties()->SetFloat("editor.shadowangle", propertyList->GetFloatPropertyValue("Shadow angle"));
            light->GetCustomProperties()->SetInt32("editor.shadowsamples", propertyList->GetFloatPropertyValue("Shadow samples"));
        }
    }

    NodesPropertyControl::OnComboIndexChanged(forList, forKey, newItemIndex, newItemKey);
}

void LightPropertyControl::OnBoolPropertyChanged(PropertyList *forList, const String &forKey, bool newValue)
{
    if("Enable" == forKey)
    {
        LightNode *light = dynamic_cast<LightNode *>(currentNode);
        light->GetCustomProperties()->SetBool("editor.staticlight.enable", newValue);
    }
    else if("Cast shadows" == forKey)
    {
        LightNode *light = dynamic_cast<LightNode *>(currentNode);
        light->GetCustomProperties()->SetBool("editor.staticlight.castshadows", newValue);
    }

    NodesPropertyControl::OnBoolPropertyChanged(forList, forKey, newValue);
}

void LightPropertyControl::OnFloatPropertyChanged(PropertyList *forList, const String &forKey, float newValue)
{
    if("property.lightnode.r" == forKey || "property.lightnode.g" == forKey || "property.lightnode.b" == forKey)
    {
        LightNode *light = dynamic_cast<LightNode *>(currentNode);
        Color color(
                    propertyList->GetFloatPropertyValue("property.lightnode.r"),
                    propertyList->GetFloatPropertyValue("property.lightnode.g"),
                    propertyList->GetFloatPropertyValue("property.lightnode.b"),
                    1.f);
        light->SetColor(color);
    }
    else if("Intensity" == forKey)
    {
        LightNode *light = dynamic_cast<LightNode *>(currentNode);
        light->GetCustomProperties()->SetFloat("editor.intensity", newValue);
    }
    else if("Shadow angle" == forKey)
    {
        LightNode *light = dynamic_cast<LightNode *>(currentNode);
        if(LightNode::TYPE_DIRECTIONAL == light->GetType())
        {
            light->GetCustomProperties()->SetFloat("editor.shadowangle", newValue);
        }
    }

    NodesPropertyControl::OnFloatPropertyChanged(forList, forKey, newValue);
}

void LightPropertyControl::OnIntPropertyChanged(PropertyList *forList, const String &forKey, int newValue)
{
    if("Shadow samples" == forKey)
    {
        LightNode *light = dynamic_cast<LightNode *>(currentNode);
        if(LightNode::TYPE_DIRECTIONAL == light->GetType())
        {
            light->GetCustomProperties()->SetInt32("editor.shadowsamples", newValue);
        }
    }
    
    NodesPropertyControl::OnIntPropertyChanged(forList, forKey, newValue);
}

