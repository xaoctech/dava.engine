#include "SpherePropertyControl.h"


SpherePropertyControl::SpherePropertyControl(const Rect & rect)
:   NodePropertyControl(rect)
{
    
}

void SpherePropertyControl::InitProperties()
{
    NodePropertyControl::InitProperties();
    
    propertyList->AddFloatProperty("Radius", 1.f, PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("r", 0.f, PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("g", 0.f, PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("b", 0.f, PropertyList::PROPERTY_IS_EDITABLE); 
    propertyList->AddFloatProperty("a", 1.f, PropertyList::PROPERTY_IS_EDITABLE); 
}

void SpherePropertyControl::SetDefaultValues()
{
    propertyList->SetStringPropertyValue("Name", "Sphere");
    propertyList->SetFloatPropertyValue("Radius", 1.f);
    propertyList->SetFloatPropertyValue("r", 0.f);
    propertyList->SetFloatPropertyValue("g", 0.f);
    propertyList->SetFloatPropertyValue("b", 0.f); 
    propertyList->SetFloatPropertyValue("a", 1.f); 
}

void SpherePropertyControl::ReadFromNode(SceneNode *sceneNode)
{
    NodePropertyControl::ReadFromNode(sceneNode);
    
    SphereNode *sphere = (SphereNode *)sceneNode;
    
    float32 radius = sphere->GetRadius();
    propertyList->SetFloatPropertyValue("Radius", radius);
    propertyList->SetFloatPropertyValue("r", sphere->GetColor().r);
    propertyList->SetFloatPropertyValue("g", sphere->GetColor().g);
    propertyList->SetFloatPropertyValue("b", sphere->GetColor().b);
    propertyList->SetFloatPropertyValue("a", sphere->GetColor().a);
}

void SpherePropertyControl::ReadToNode(SceneNode *sceneNode)
{
    NodePropertyControl::ReadToNode(sceneNode);
    
    SphereNode *sphere = (SphereNode *)sceneNode;
    Color color(
                propertyList->GetFloatPropertyValue("r"),
                propertyList->GetFloatPropertyValue("g"),
                propertyList->GetFloatPropertyValue("b"),
                propertyList->GetFloatPropertyValue("a"));
    
    float32 radius = propertyList->GetFloatPropertyValue("Radius");
    
    sphere->CreateSphere(radius, color);
}


