#include "CreateServiceNodeDialog.h"
#include "ControlsFactory.h"

CreateServiceNodeDialog::CreateServiceNodeDialog(const Rect & rect)
    :   CreateNodeDialog(rect)
{
    SetHeader(L"Create Service Node");
}
    
CreateServiceNodeDialog::~CreateServiceNodeDialog()
{
}

void CreateServiceNodeDialog::InitializeProperties()
{
    propertyList->AddTextProperty("Name", "Service Node", PropertyList::PROPERTY_IS_EDITABLE);
}

void CreateServiceNodeDialog::CreateNode()
{
    SafeRelease(sceneNode);
    sceneNode = new SceneNode(scene);
    
    sceneNode->SetName(propertyList->GetTextPropertyValue("Name"));
}

void CreateServiceNodeDialog::ClearPropertyValues()
{
    propertyList->SetTextPropertyValue("Name", "Service Node");
}
