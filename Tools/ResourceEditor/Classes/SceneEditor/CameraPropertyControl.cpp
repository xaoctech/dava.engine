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

    propertyList->AddFloatProperty("position.x", 0.f, PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("position.y", 0.f, PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("position.z", 0.f, PropertyList::PROPERTY_IS_EDITABLE);

    propertyList->AddFloatProperty("target.x", 0.f, PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("target.y", 0.f, PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("target.z", 1.f, PropertyList::PROPERTY_IS_EDITABLE);
}

void CameraPropertyControl::SetDefaultValues()
{
    propertyList->SetStringPropertyValue("Name", "Camera");
    propertyList->SetFloatPropertyValue("Fov", 70.f);
    propertyList->SetFloatPropertyValue("zNear", 1.f);
    propertyList->SetFloatPropertyValue("zFar", 5000.f);
    propertyList->SetBoolPropertyValue("isOrtho", false);

    propertyList->SetFloatPropertyValue("position.x", 0.f);
    propertyList->SetFloatPropertyValue("position.y", 0.f);
    propertyList->SetFloatPropertyValue("position.z", 0.f);

    propertyList->SetFloatPropertyValue("target.x", 0.f);
    propertyList->SetFloatPropertyValue("target.y", 0.f);
    propertyList->SetFloatPropertyValue("target.z", 1.f);
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
    propertyList->SetFloatPropertyValue("position.x", pos.x);
    propertyList->SetFloatPropertyValue("position.y", pos.y);
    propertyList->SetFloatPropertyValue("position.z", pos.z);
    
    Vector3 target = camera->GetTarget();
    propertyList->SetFloatPropertyValue("target.x", target.x);
    propertyList->SetFloatPropertyValue("target.y", target.y);
    propertyList->SetFloatPropertyValue("target.z", target.z);

}

void CameraPropertyControl::ReadToNode(SceneNode *sceneNode)
{
    NodePropertyControl::ReadToNode(sceneNode);
    
    Camera *camera = (Camera *)sceneNode;
    
    camera->Setup(
                                 propertyList->GetFloatPropertyValue("Fov"),
                                 320.0f / 480.0f,
                                 propertyList->GetFloatPropertyValue("zNear"),
                                 propertyList->GetFloatPropertyValue("zFar"),
                                 propertyList->GetBoolPropertyValue("isOrtho"));

    camera->SetPosition(Vector3(
                               propertyList->GetFloatPropertyValue("position.x"),
                               propertyList->GetFloatPropertyValue("position.y"),
                               propertyList->GetFloatPropertyValue("position.z")));
    camera->SetTarget(Vector3(
                                propertyList->GetFloatPropertyValue("target.x"),
                                propertyList->GetFloatPropertyValue("target.y"),
                                propertyList->GetFloatPropertyValue("target.z")));
}


