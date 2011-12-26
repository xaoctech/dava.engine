#include "CreateCameraDialog.h"
#include "ControlsFactory.h"

CreateCameraDialog::CreateCameraDialog(const Rect & rect)
    :   CreateNodeDialog(rect)
{
    SetHeader(L"Create Camera");
}
    
CreateCameraDialog::~CreateCameraDialog()
{
}

void CreateCameraDialog::InitializeProperties()
{
    propertyList->AddTextProperty("Name", "Camera", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("Fov", 70.f, PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("zNear", 1.f, PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("zFar", 5000.f, PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddBoolProperty("isOrtho", false, PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("x", 0.f, PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("y", 0.f, PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("z", 0.f, PropertyList::PROPERTY_IS_EDITABLE);
}

void CreateCameraDialog::CreateNode()
{
    SafeRelease(sceneNode);
    sceneNode = new Camera(scene);
    
    sceneNode->SetName(propertyList->GetTextPropertyValue("Name"));

    ((Camera *)sceneNode)->Setup(
                                 propertyList->GetFloatPropertyValue("Fov"),
                                 320.0f / 480.0f,
                                 propertyList->GetFloatPropertyValue("zNear"),
                                 propertyList->GetFloatPropertyValue("zFar"),
                                 propertyList->GetBoolPropertyValue("isOrtho"));
    ((Camera *)sceneNode)->SetPosition(Vector3(
                                               propertyList->GetFloatPropertyValue("x"),
                                               propertyList->GetFloatPropertyValue("y"),
                                               propertyList->GetFloatPropertyValue("z")));
}

void CreateCameraDialog::ClearPropertyValues()
{
    propertyList->SetTextPropertyValue("Name", "Camera");
    propertyList->SetFloatPropertyValue("Fov", 70.f);
    propertyList->SetFloatPropertyValue("zNear", 1.f);
    propertyList->SetFloatPropertyValue("zFar", 5000.f);
    propertyList->SetBoolPropertyValue("isOrtho", false);
    propertyList->SetFloatPropertyValue("x", 0.f);
    propertyList->SetFloatPropertyValue("y", 0.f);
    propertyList->SetFloatPropertyValue("z", 0.f);
}
