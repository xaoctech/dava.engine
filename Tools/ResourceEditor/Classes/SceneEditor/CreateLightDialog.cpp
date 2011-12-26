#include "CreateLightDialog.h"
#include "ControlsFactory.h"

CreateLightDialog::CreateLightDialog(const Rect & rect)
    :   CreateNodeDialog(rect)
{
    SetHeader(L"Create Light");
}
    
CreateLightDialog::~CreateLightDialog()
{
}

void CreateLightDialog::InitializeProperties()
{
    propertyList->AddTextProperty("Name", "Light", PropertyList::PROPERTY_IS_EDITABLE);
//    propertyList->AddFloatProperty("Type", 1.f, PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("r", 1.f, PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("g", 1.f, PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("b", 1.f, PropertyList::PROPERTY_IS_EDITABLE); 
    propertyList->AddFloatProperty("a", 1.f, PropertyList::PROPERTY_IS_EDITABLE); 
 
}

void CreateLightDialog::CreateNode()
{
    SafeRelease(sceneNode);
    sceneNode = new LightNode(scene);
    
    sceneNode->SetName(propertyList->GetTextPropertyValue("Name"));
    
    Color color(
                propertyList->GetFloatPropertyValue("r"),
                propertyList->GetFloatPropertyValue("g"),
                propertyList->GetFloatPropertyValue("b"),
                propertyList->GetFloatPropertyValue("a"));
    
    int32 type = LightNode::ET_DIRECTIONAL; //propertyList->GetFloatPropertyValue("Type");

    ((LightNode *)sceneNode)->SetColor(color);
    ((LightNode *)sceneNode)->SetType((LightNode::eType)type);
}

void CreateLightDialog::ClearPropertyValues()
{
    propertyList->SetTextPropertyValue("Name", "Light");
//    propertyList->SetFloatPropertyValue("Type", 1.f);
    propertyList->SetFloatPropertyValue("r", 1.f);
    propertyList->SetFloatPropertyValue("g", 1.f);
    propertyList->SetFloatPropertyValue("b", 1.f); 
    propertyList->SetFloatPropertyValue("a", 1.f); 
}
