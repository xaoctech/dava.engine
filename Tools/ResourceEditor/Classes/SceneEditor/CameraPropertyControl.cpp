#include "CameraPropertyControl.h"

CameraPropertyControl::CameraPropertyControl(const Rect & rect)
:   NodePropertyControl(rect)
{
    
}

void CameraPropertyControl::InitProperties()
{
    NodePropertyControl::InitProperties();
    
    propertyList->AddFloatProperty("Fov", 70.f, PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("zNear", 1.f, PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("zFar", 5000.f, PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddBoolProperty("isOrtho", false, PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("x", 0.f, PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("y", 0.f, PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("z", 0.f, PropertyList::PROPERTY_IS_EDITABLE);
}

void CameraPropertyControl::SetDefaultValues()
{
    propertyList->SetStringPropertyValue("Name", "Camera");
    propertyList->SetFloatPropertyValue("Fov", 70.f);
    propertyList->SetFloatPropertyValue("zNear", 1.f);
    propertyList->SetFloatPropertyValue("zFar", 5000.f);
    propertyList->SetBoolPropertyValue("isOrtho", false);
    propertyList->SetFloatPropertyValue("x", 0.f);
    propertyList->SetFloatPropertyValue("y", 0.f);
    propertyList->SetFloatPropertyValue("z", 0.f);
}

void CameraPropertyControl::ReadFromNode(SceneNode *sceneNode)
{
    NodePropertyControl::ReadFromNode(sceneNode);
    
    Camera *camera = (Camera *)sceneNode;
    
    propertyList->SetFloatPropertyValue("Fov", camera->GetFOV());
    propertyList->SetFloatPropertyValue("zNear", camera->GetZNear());
    propertyList->SetFloatPropertyValue("zFar", camera->GetZFar());
    propertyList->SetBoolPropertyValue("isOrtho", camera->GetIsOrtho());

    Vector3 pos = camera->GetPosition();
    propertyList->SetFloatPropertyValue("x", pos.x);
    propertyList->SetFloatPropertyValue("y", pos.y);
    propertyList->SetFloatPropertyValue("z", pos.z);
}

void CameraPropertyControl::ReadToNode(SceneNode *sceneNode)
{
    NodePropertyControl::ReadToNode(sceneNode);
    
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


