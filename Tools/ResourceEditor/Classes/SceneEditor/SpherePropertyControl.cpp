#include "SpherePropertyControl.h"


SpherePropertyControl::SpherePropertyControl(const Rect & rect, bool showMatrix)
:   NodePropertyControl(rect, showMatrix)
{
    
}

void SpherePropertyControl::InitProperties()
{
    NodePropertyControl::InitProperties();
    
    propertyList->AddFloatProperty("Radius", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("r", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("g", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("b", PropertyList::PROPERTY_IS_EDITABLE); 
    propertyList->AddFloatProperty("a", PropertyList::PROPERTY_IS_EDITABLE); 

    propertyList->SetStringPropertyValue("Name", "Sphere");
    propertyList->SetFloatPropertyValue("Radius", 1.f);
    propertyList->SetFloatPropertyValue("r", 0.f);
    propertyList->SetFloatPropertyValue("g", 0.f);
    propertyList->SetFloatPropertyValue("b", 0.f); 
    propertyList->SetFloatPropertyValue("a", 1.f); 
}


void SpherePropertyControl::ReadFrom(SceneNode *sceneNode)
{
    NodePropertyControl::ReadFrom(sceneNode);
    
    SphereNode *sphere = (SphereNode *)sceneNode;
    
    float32 radius = sphere->GetRadius();
    propertyList->SetFloatPropertyValue("Radius", radius);
    propertyList->SetFloatPropertyValue("r", sphere->GetColor().r);
    propertyList->SetFloatPropertyValue("g", sphere->GetColor().g);
    propertyList->SetFloatPropertyValue("b", sphere->GetColor().b);
    propertyList->SetFloatPropertyValue("a", sphere->GetColor().a);
}

void SpherePropertyControl::WriteTo(SceneNode *sceneNode)
{
    NodePropertyControl::WriteTo(sceneNode);
    
    SphereNode *sphere = (SphereNode *)sceneNode;
    Color color(
                propertyList->GetFloatPropertyValue("r"),
                propertyList->GetFloatPropertyValue("g"),
                propertyList->GetFloatPropertyValue("b"),
                propertyList->GetFloatPropertyValue("a"));
    
    float32 radius = propertyList->GetFloatPropertyValue("Radius");
    
    sphere->SetColor(color);
    sphere->SetRadius(radius);
//    sphere->CreateSphere(radius, color);
}


