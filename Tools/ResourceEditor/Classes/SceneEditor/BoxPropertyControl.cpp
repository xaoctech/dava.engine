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

    propertyList->AddSection("Cube", GetHeaderState("Cube", true));
    
    propertyList->AddFloatProperty("Length", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("Width", PropertyList::PROPERTY_IS_EDITABLE); 
    propertyList->AddFloatProperty("Depth", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("r", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("g", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("b", PropertyList::PROPERTY_IS_EDITABLE); 
    propertyList->AddFloatProperty("a", PropertyList::PROPERTY_IS_EDITABLE); 
    
    Vector3 size = cube->GetSize();
    propertyList->SetFloatPropertyValue("Length", size.x);
    propertyList->SetFloatPropertyValue("Width", size.y); 
    propertyList->SetFloatPropertyValue("Depth", size.z);
    propertyList->SetFloatPropertyValue("r", cube->GetColor().r);
    propertyList->SetFloatPropertyValue("g", cube->GetColor().g);
    propertyList->SetFloatPropertyValue("b", cube->GetColor().b);
    propertyList->SetFloatPropertyValue("a", cube->GetColor().a);
}

void BoxPropertyControl::WriteTo(SceneNode * sceneNode)
{
	MeshInstancePropertyControl::WriteTo(sceneNode);

    CubeNode *cube = dynamic_cast<CubeNode *> (sceneNode);
	DVASSERT(cube);

    Color color(
                propertyList->GetFloatPropertyValue("r"),
                propertyList->GetFloatPropertyValue("g"),
                propertyList->GetFloatPropertyValue("b"),
                propertyList->GetFloatPropertyValue("a"));
    
    Vector3 size(
                 propertyList->GetFloatPropertyValue("Length"),
                 propertyList->GetFloatPropertyValue("Width"),
                 propertyList->GetFloatPropertyValue("Depth"));
    
    cube->SetSize(size);
    cube->SetColor(color);
}

void BoxPropertyControl::OnFloatPropertyChanged(PropertyList *forList, const String &forKey, float newValue)
{
    if("r" == forKey || "g" == forKey || "b" == forKey || "a" == forKey)
    {
        CubeNode *cube = dynamic_cast<CubeNode *> (currentNode);
        Color color(
                    propertyList->GetFloatPropertyValue("r"),
                    propertyList->GetFloatPropertyValue("g"),
                    propertyList->GetFloatPropertyValue("b"),
                    propertyList->GetFloatPropertyValue("a"));

        cube->SetColor(color);
    }
    else if("Length" == forKey || "Width" == forKey || "Depth" == forKey)
    {
        CubeNode *cube = dynamic_cast<CubeNode *> (currentNode);
        Vector3 size(
                     propertyList->GetFloatPropertyValue("Length"),
                     propertyList->GetFloatPropertyValue("Width"),
                     propertyList->GetFloatPropertyValue("Depth"));
        
        cube->SetSize(size);
    }

    MeshInstancePropertyControl::OnFloatPropertyChanged(forList, forKey, newValue);
}
