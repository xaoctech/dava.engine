#include "LightPropertyControl.h"

LightPropertyControl::LightPropertyControl(const Rect & rect, bool showMatrix)
:   NodePropertyControl(rect, showMatrix)
{
    types.push_back("Directional");
    types.push_back("Spot");
    types.push_back("Point");
}

void LightPropertyControl::InitProperties()
{
    NodePropertyControl::InitProperties();
    
    propertyList->AddComboProperty("Type", types);
    propertyList->AddFloatProperty("r", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("g", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("b", PropertyList::PROPERTY_IS_EDITABLE); 
    propertyList->AddFloatProperty("a", PropertyList::PROPERTY_IS_EDITABLE); 
    
    propertyList->SetStringPropertyValue("Name", "LightNode");
    propertyList->SetComboPropertyIndex("Type", 0);
    propertyList->SetFloatPropertyValue("r", 1.f);
    propertyList->SetFloatPropertyValue("g", 1.f);
    propertyList->SetFloatPropertyValue("b", 1.f);
    propertyList->SetFloatPropertyValue("a", 1.f);

}


void LightPropertyControl::ReadFrom(SceneNode *sceneNode)
{
    NodePropertyControl::ReadFrom(sceneNode);
    
    LightNode *light = (LightNode *)sceneNode;

    propertyList->SetComboPropertyIndex("Type", (int32)light->GetType());
    propertyList->SetFloatPropertyValue("r", light->GetColor().r);
    propertyList->SetFloatPropertyValue("g", light->GetColor().g);
    propertyList->SetFloatPropertyValue("b", light->GetColor().b);
    propertyList->SetFloatPropertyValue("a", light->GetColor().a);
}

void LightPropertyControl::WriteTo(SceneNode *sceneNode)
{
    NodePropertyControl::WriteTo(sceneNode);
    
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


