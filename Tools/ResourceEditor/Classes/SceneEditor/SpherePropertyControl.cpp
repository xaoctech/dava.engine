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
    
    propertyList->AddSection("property.spherenode.sphere", GetHeaderState("property.spherenode.sphere", true));
    
    propertyList->AddFloatProperty("property.spherenode.radius", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("property.spherenode.r", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("property.spherenode.g", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("property.spherenode.b", PropertyList::PROPERTY_IS_EDITABLE); 
    propertyList->AddFloatProperty("property.spherenode.a", PropertyList::PROPERTY_IS_EDITABLE); 
    
    float32 radius = sphere->GetRadius();
    propertyList->SetFloatPropertyValue("property.spherenode.radius", radius);
    propertyList->SetFloatPropertyValue("property.spherenode.r", sphere->GetColor().r);
    propertyList->SetFloatPropertyValue("property.spherenode.g", sphere->GetColor().g);
    propertyList->SetFloatPropertyValue("property.spherenode.b", sphere->GetColor().b);
    propertyList->SetFloatPropertyValue("property.spherenode.a", sphere->GetColor().a);
}

void SpherePropertyControl::OnFloatPropertyChanged(PropertyList *forList, const String &forKey, float newValue)
{
    if(     "property.spherenode.r" == forKey 
       ||   "property.spherenode.g" == forKey 
       ||   "property.spherenode.b" == forKey 
       ||   "property.spherenode.a" == forKey)
    {
        SphereNode *sphere = dynamic_cast<SphereNode *> (currentNode);
        Color color(
                    propertyList->GetFloatPropertyValue("property.spherenode.r"),
                    propertyList->GetFloatPropertyValue("property.spherenode.g"),
                    propertyList->GetFloatPropertyValue("property.spherenode.b"),
                    propertyList->GetFloatPropertyValue("property.spherenode.a"));
        
        sphere->SetColor(color);
    }
    else if("property.spherenode.radius" == forKey)
    {
        SphereNode *sphere = dynamic_cast<SphereNode *> (currentNode);
        sphere->SetRadius(newValue);
    }

    MeshInstancePropertyControl::OnFloatPropertyChanged(forList, forKey, newValue);
}
