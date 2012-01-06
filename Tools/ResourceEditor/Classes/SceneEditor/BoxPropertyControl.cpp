#include "BoxPropertyControl.h"


BoxPropertyControl::BoxPropertyControl(const Rect & rect, bool showMatrix)
:   NodePropertyControl(rect, showMatrix)
{
    
}

void BoxPropertyControl::InitProperties()
{
    NodePropertyControl::InitProperties();
    
    propertyList->AddFloatProperty("Length", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("Width", PropertyList::PROPERTY_IS_EDITABLE); 
    propertyList->AddFloatProperty("Depth", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("r", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("g", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("b", PropertyList::PROPERTY_IS_EDITABLE); 
    propertyList->AddFloatProperty("a", PropertyList::PROPERTY_IS_EDITABLE); 

    propertyList->SetStringPropertyValue("Name", "Box");
    propertyList->SetFloatPropertyValue("Length", 1.f);
    propertyList->SetFloatPropertyValue("Width", 1.f); 
    propertyList->SetFloatPropertyValue("Depth", 1.f);
    propertyList->SetFloatPropertyValue("r", 0.f);
    propertyList->SetFloatPropertyValue("g", 0.f);
    propertyList->SetFloatPropertyValue("b", 0.f); 
    propertyList->SetFloatPropertyValue("a", 1.f); 
}


void BoxPropertyControl::ReadFrom(SceneNode *sceneNode)
{
    NodePropertyControl::ReadFrom(sceneNode);
    
    CubeNode *cube = (CubeNode *)sceneNode;
    
    Vector3 size = cube->GetSize();
    propertyList->SetFloatPropertyValue("Length", size.x);
    propertyList->SetFloatPropertyValue("Width", size.y); 
    propertyList->SetFloatPropertyValue("Depth", size.z);
    propertyList->SetFloatPropertyValue("r", cube->GetColor().r);
    propertyList->SetFloatPropertyValue("g", cube->GetColor().g);
    propertyList->SetFloatPropertyValue("b", cube->GetColor().b);
    propertyList->SetFloatPropertyValue("a", cube->GetColor().a);
}

void BoxPropertyControl::WriteTo(SceneNode *sceneNode)
{
    NodePropertyControl::WriteTo(sceneNode);
    
    CubeNode *cube = (CubeNode *)sceneNode;
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
    
//    cube->CreateCube(size, color);
}


