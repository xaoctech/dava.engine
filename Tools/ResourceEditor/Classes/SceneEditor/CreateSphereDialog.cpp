#include "CreateSphereDialog.h"
#include "ControlsFactory.h"

CreateSphereDialog::CreateSphereDialog(const Rect & rect)
    :   CreateNodeDialog(rect)
{
    SetHeader(L"Create Sphere");
}
    
CreateSphereDialog::~CreateSphereDialog()
{
}

void CreateSphereDialog::InitializeProperties()
{
    propertyList->AddTextProperty("Name", "Sphere", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("Radius", 1.f, PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("r", 0.f, PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("g", 0.f, PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("b", 0.f, PropertyList::PROPERTY_IS_EDITABLE); 
    propertyList->AddFloatProperty("a", 1.f, PropertyList::PROPERTY_IS_EDITABLE); 
}

void CreateSphereDialog::CreateNode()
{
    SafeRelease(sceneNode);
    sceneNode = new SphereNode(scene);
    
    sceneNode->SetName(propertyList->GetTextPropertyValue("Name"));
    
    Color color(
                propertyList->GetFloatPropertyValue("r"),
                propertyList->GetFloatPropertyValue("g"),
                propertyList->GetFloatPropertyValue("b"),
                propertyList->GetFloatPropertyValue("a"));
    
    float32 radius = propertyList->GetFloatPropertyValue("Radius");
    
    ((SphereNode *)sceneNode)->CreateSphere(radius, color);
}

void CreateSphereDialog::ClearPropertyValues()
{
    propertyList->SetTextPropertyValue("Name", "Sphere");
    propertyList->SetFloatPropertyValue("Radius", 1.f);
    propertyList->SetFloatPropertyValue("r", 0.f);
    propertyList->SetFloatPropertyValue("g", 0.f);
    propertyList->SetFloatPropertyValue("b", 0.f);
    propertyList->SetFloatPropertyValue("a", 1.f);
}