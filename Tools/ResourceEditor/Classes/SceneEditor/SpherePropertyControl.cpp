#include "SpherePropertyControl.h"


SpherePropertyControl::SpherePropertyControl(const Rect & rect, bool createNodeProperties)
:	MeshInstancePropertyControl(rect, createNodeProperties)
{
}

SpherePropertyControl::~SpherePropertyControl()
{

}

void SpherePropertyControl::ReadFrom(SceneNode * sceneNode)
{
	MeshInstancePropertyControl::ReadFrom(sceneNode);

    SphereNode *sphere = dynamic_cast<SphereNode *> (sceneNode);
	DVASSERT(sphere);
    
    propertyList->AddSection("Sphere", GetHeaderState("Sphere", true));
    
    propertyList->AddFloatProperty("Radius", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("r", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("g", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("b", PropertyList::PROPERTY_IS_EDITABLE); 
    propertyList->AddFloatProperty("a", PropertyList::PROPERTY_IS_EDITABLE); 
    
    float32 radius = sphere->GetRadius();
    propertyList->SetFloatPropertyValue("Radius", radius);
    propertyList->SetFloatPropertyValue("r", sphere->GetColor().r);
    propertyList->SetFloatPropertyValue("g", sphere->GetColor().g);
    propertyList->SetFloatPropertyValue("b", sphere->GetColor().b);
    propertyList->SetFloatPropertyValue("a", sphere->GetColor().a);
}

void SpherePropertyControl::WriteTo(SceneNode * sceneNode)
{
	MeshInstancePropertyControl::WriteTo(sceneNode);

    SphereNode *sphere = dynamic_cast<SphereNode *> (sceneNode);
	DVASSERT(sphere);

    Color color(
                propertyList->GetFloatPropertyValue("r"),
                propertyList->GetFloatPropertyValue("g"),
                propertyList->GetFloatPropertyValue("b"),
                propertyList->GetFloatPropertyValue("a"));
    
    float32 radius = propertyList->GetFloatPropertyValue("Radius");
    
    sphere->SetColor(color);
    sphere->SetRadius(radius);
}

void SpherePropertyControl::OnFloatPropertyChanged(PropertyList *forList, const String &forKey, float newValue)
{
    if("r" == forKey || "g" == forKey || "b" == forKey || "a" == forKey)
    {
        SphereNode *sphere = dynamic_cast<SphereNode *> (currentNode);
        Color color(
                    propertyList->GetFloatPropertyValue("r"),
                    propertyList->GetFloatPropertyValue("g"),
                    propertyList->GetFloatPropertyValue("b"),
                    propertyList->GetFloatPropertyValue("a"));
        
        sphere->SetColor(color);
    }
    else if("Radius" == forKey)
    {
        SphereNode *sphere = dynamic_cast<SphereNode *> (currentNode);
        sphere->SetRadius(newValue);
    }

    MeshInstancePropertyControl::OnFloatPropertyChanged(forList, forKey, newValue);
}
