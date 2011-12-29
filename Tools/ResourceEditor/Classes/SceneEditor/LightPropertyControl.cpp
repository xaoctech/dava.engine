#include "LightPropertyControl.h"

LightPropertyControl::LightPropertyControl(const Rect & rect)
:   NodePropertyControl(rect)
{
    
}

void LightPropertyControl::InitProperties()
{
    NodePropertyControl::InitProperties();
    
//    propertyList->AddFloatProperty("Type", 1.f, PropertyList::PROPERTY_IS_EDITABLE);
    
    propertyList->AddFloatProperty("r", 1.f, PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("g", 1.f, PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("b", 1.f, PropertyList::PROPERTY_IS_EDITABLE); 
    propertyList->AddFloatProperty("a", 1.f, PropertyList::PROPERTY_IS_EDITABLE); 
}

void LightPropertyControl::SetDefaultValues()
{
    propertyList->SetStringPropertyValue("Name", "LightNode");
//    propertyList->SetFloatPropertyValue("Type", 1.f);

    propertyList->SetFloatPropertyValue("r", 1.f);
    propertyList->SetFloatPropertyValue("g", 1.f);
    propertyList->SetFloatPropertyValue("b", 1.f);
    propertyList->SetFloatPropertyValue("a", 1.f);
}

void LightPropertyControl::ReadFromNode(SceneNode *sceneNode)
{
    NodePropertyControl::ReadFromNode(sceneNode);
    
    LightNode *light = (LightNode *)sceneNode;

//    propertyList->SetFloatPropertyValue("Type", light->GetType());
    
    propertyList->SetFloatPropertyValue("r", light->GetColor().r);
    propertyList->SetFloatPropertyValue("g", light->GetColor().g);
    propertyList->SetFloatPropertyValue("b", light->GetColor().b);
    propertyList->SetFloatPropertyValue("a", light->GetColor().a);
}

void LightPropertyControl::ReadToNode(SceneNode *sceneNode)
{
    NodePropertyControl::ReadToNode(sceneNode);
    
    LightNode *light = (LightNode *)sceneNode;

    
    Color color(
                propertyList->GetFloatPropertyValue("r"),
                propertyList->GetFloatPropertyValue("g"),
                propertyList->GetFloatPropertyValue("b"),
                propertyList->GetFloatPropertyValue("a"));
    
    int32 type = LightNode::ET_DIRECTIONAL; //propertyList->GetFloatPropertyValue("Type");
    
    light->SetColor(color);
    light->SetType((LightNode::eType)type);
}


