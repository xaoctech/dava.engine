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
    propertyList->AddFloatProperty("property.cubenode.r", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("property.cubenode.g", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("property.cubenode.b", PropertyList::PROPERTY_IS_EDITABLE); 
    propertyList->AddFloatProperty("property.cubenode.a", PropertyList::PROPERTY_IS_EDITABLE); 
    
    Vector3 size = cube->GetSize();
    propertyList->SetFloatPropertyValue("property.cubenode.length", size.x);
    propertyList->SetFloatPropertyValue("property.cubenode.width", size.y); 
    propertyList->SetFloatPropertyValue("property.cubenode.depth", size.z);
    propertyList->SetFloatPropertyValue("property.cubenode.r", cube->GetColor().r);
    propertyList->SetFloatPropertyValue("property.cubenode.g", cube->GetColor().g);
    propertyList->SetFloatPropertyValue("property.cubenode.b", cube->GetColor().b);
    propertyList->SetFloatPropertyValue("property.cubenode.a", cube->GetColor().a);
}

void BoxPropertyControl::OnFloatPropertyChanged(PropertyList *forList, const String &forKey, float newValue)
{
    if(     "property.cubenode.r" == forKey 
       ||   "property.cubenode.g" == forKey 
       ||   "property.cubenode.b" == forKey 
       ||   "property.cubenode.a" == forKey)
    {
        CubeNode *cube = dynamic_cast<CubeNode *> (currentNode);
        Color color(
                    propertyList->GetFloatPropertyValue("property.cubenode.r"),
                    propertyList->GetFloatPropertyValue("property.cubenode.g"),
                    propertyList->GetFloatPropertyValue("property.cubenode.b"),
                    propertyList->GetFloatPropertyValue("property.cubenode.a"));

        cube->SetColor(color);
    }
    else if(    "property.cubenode.length" == forKey 
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
