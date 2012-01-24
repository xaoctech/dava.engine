#include "BoxPropertyControl.h"


BoxPropertyControl::BoxPropertyControl(const Rect & rect, bool createNodeProperties)
:	MeshInstancePropertyControl(rect, createNodeProperties)
{
}

BoxPropertyControl::~BoxPropertyControl()
{
}

void BoxPropertyControl::ReadFrom(SceneNode * sceneNode)
{
	MeshInstancePropertyControl::ReadFrom(sceneNode);

    CubeNode *cube = dynamic_cast<CubeNode *> (sceneNode);
	DVASSERT(cube);

    propertyList->AddSection("property.cubenode.cube", GetHeaderState("property.cubenode.cube", true));
    
    propertyList->AddFloatProperty("property.cubenode.length", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("property.cubenode.width", PropertyList::PROPERTY_IS_EDITABLE); 
    propertyList->AddFloatProperty("property.cubenode.depth", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddColorProperty("property.cubenode.color");
    
    Vector3 size = cube->GetSize();
    propertyList->SetFloatPropertyValue("property.cubenode.length", size.x);
    propertyList->SetFloatPropertyValue("property.cubenode.width", size.y); 
    propertyList->SetFloatPropertyValue("property.cubenode.depth", size.z);
    propertyList->SetColorPropertyValue("property.cubenode.color", cube->GetColor());
}

void BoxPropertyControl::OnFloatPropertyChanged(PropertyList *forList, const String &forKey, float newValue)
{
    if(    "property.cubenode.length" == forKey 
        ||  "property.cubenode.width" == forKey 
        ||  "property.cubenode.depth" == forKey)
    {
        CubeNode *cube = dynamic_cast<CubeNode *> (currentNode);
        Vector3 size(
                     propertyList->GetFloatPropertyValue("property.cubenode.length"),
                     propertyList->GetFloatPropertyValue("property.cubenode.width"),
                     propertyList->GetFloatPropertyValue("property.cubenode.depth"));
        
        cube->SetSize(size);
    }

    MeshInstancePropertyControl::OnFloatPropertyChanged(forList, forKey, newValue);
}

void BoxPropertyControl::OnColorPropertyChanged(PropertyList *forList, const String &forKey, const Color& newColor)
{
    if("property.cubenode.color" == forKey)
    {
        CubeNode *cube = dynamic_cast<CubeNode *> (currentNode);
        cube->SetColor(newColor);
    }
}
