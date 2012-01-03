#include "LightPropertyControl.h"

LightPropertyControl::LightPropertyControl(const Rect & rect)
:   NodePropertyControl(rect)
{
    types.push_back("Directional");
    types.push_back("Spot");
    types.push_back("Point");
}

void LightPropertyControl::InitProperties()
{
    NodePropertyControl::InitProperties();
    
    propertyList->AddComboProperty("Type", types, 1);
    propertyList->AddFloatProperty("r", 1.f, PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("g", 1.f, PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("b", 1.f, PropertyList::PROPERTY_IS_EDITABLE); 
    propertyList->AddFloatProperty("a", 1.f, PropertyList::PROPERTY_IS_EDITABLE); 
}

void LightPropertyControl::SetDefaultValues()
{
    propertyList->SetStringPropertyValue("Name", "LightNode");
    propertyList->SetComboPropertyValue("Type", 0);
    propertyList->SetFloatPropertyValue("r", 1.f);
    propertyList->SetFloatPropertyValue("g", 1.f);
    propertyList->SetFloatPropertyValue("b", 1.f);
    propertyList->SetFloatPropertyValue("a", 1.f);
}

void LightPropertyControl::ReadFromNode(SceneNode *sceneNode)
{
    NodePropertyControl::ReadFromNode(sceneNode);
    
    LightNode *light = (LightNode *)sceneNode;

    propertyList->SetComboPropertyValue("Type", (int32)light->GetType());
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
    
    int32 type = LightNode::ET_DIRECTIONAL; 
    String typeName = propertyList->GetComboPropertyValue("Type");
    for(int32 i = 0; i < types.size(); ++i)
    {
        if(typeName == types[i])
        {
            type = i;
            break;
        }
    }

    
    light->SetColor(color);
    light->SetType((LightNode::eType)type);
}


