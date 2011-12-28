#include "CreateBoxDialog.h"
#include "ControlsFactory.h"

CreateBoxDialog::CreateBoxDialog(const Rect & rect)
    :   CreateNodeDialog(rect)
{
    SetHeader(L"Create Box");
}
    
CreateBoxDialog::~CreateBoxDialog()
{
}

void CreateBoxDialog::InitializeProperties()
{
    propertyList->AddStringProperty("Name", "Camera", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("Length", 1.f, PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("Width", 1.f, PropertyList::PROPERTY_IS_EDITABLE); 
    propertyList->AddFloatProperty("Depth", 1.f, PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("r", 0.f, PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("g", 0.f, PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("b", 0.f, PropertyList::PROPERTY_IS_EDITABLE); 
    propertyList->AddFloatProperty("a", 1.f, PropertyList::PROPERTY_IS_EDITABLE); 
}

void CreateBoxDialog::CreateNode()
{
    SafeRelease(sceneNode);
    sceneNode = new CubeNode(scene);
    
    sceneNode->SetName(propertyList->GetStringPropertyValue("Name"));
    
    Color color(
                propertyList->GetFloatPropertyValue("r"),
                propertyList->GetFloatPropertyValue("g"),
                propertyList->GetFloatPropertyValue("b"),
                propertyList->GetFloatPropertyValue("a"));

    Vector3 size(
                 propertyList->GetFloatPropertyValue("Length"),
                 propertyList->GetFloatPropertyValue("Width"),
                 propertyList->GetFloatPropertyValue("Depth"));
    ((CubeNode *)sceneNode)->CreateCube(size, color);
    
}

void CreateBoxDialog::ClearPropertyValues()
{
    propertyList->SetStringPropertyValue("Name", "Box");
    propertyList->SetFloatPropertyValue("Length", 1.f);
    propertyList->SetFloatPropertyValue("Width", 1.f); 
    propertyList->SetFloatPropertyValue("Depth", 1.f);
    propertyList->SetFloatPropertyValue("r", 0.f);
    propertyList->SetFloatPropertyValue("g", 0.f);
    propertyList->SetFloatPropertyValue("b", 0.f); 
    propertyList->SetFloatPropertyValue("a", 1.f); 
}