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
    propertyList->AddColorProperty("property.spherenode.color");
    
    float32 radius = sphere->GetRadius();
    propertyList->SetFloatPropertyValue("property.spherenode.radius", radius);
    propertyList->SetColorPropertyValue("property.spherenode.color", sphere->GetColor());
}

void SpherePropertyControl::OnFloatPropertyChanged(PropertyList *forList, const String &forKey, float newValue)
{
    if("property.spherenode.radius" == forKey)
    {
        SphereNode *sphere = dynamic_cast<SphereNode *> (currentNode);
        sphere->SetRadius(newValue);
    }

    MeshInstancePropertyControl::OnFloatPropertyChanged(forList, forKey, newValue);
}

void SpherePropertyControl::OnColorPropertyChanged(PropertyList *forList, const String &forKey, const Color& newColor)
{
    if("property.cubenode.color" == forKey)
    {
        SphereNode *sphere = dynamic_cast<SphereNode *> (currentNode);
        sphere->SetColor(newColor);
    }
}
