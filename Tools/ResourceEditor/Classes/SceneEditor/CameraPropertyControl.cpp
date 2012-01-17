#include "CameraPropertyControl.h"


CameraPropertyControl::CameraPropertyControl(const Rect & rect, bool createNodeProperties)
:	NodesPropertyControl(rect, createNodeProperties)
{
}

CameraPropertyControl::~CameraPropertyControl()
{

}

void CameraPropertyControl::ReadFrom(SceneNode * sceneNode)
{
	NodesPropertyControl::ReadFrom(sceneNode);

    Camera *camera = dynamic_cast<Camera*> (sceneNode);
	DVASSERT(camera);

    propertyList->AddSection("Camera", GetHeaderState("Camera", true));
        
    propertyList->AddFloatProperty("Fov", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("zNear", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("zFar", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddBoolProperty("isOrtho", PropertyList::PROPERTY_IS_EDITABLE);
    
    propertyList->AddFloatProperty("position.x", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("position.y", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("position.z", PropertyList::PROPERTY_IS_EDITABLE);
    
    propertyList->AddFloatProperty("target.x", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("target.y", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("target.z", PropertyList::PROPERTY_IS_EDITABLE);
    
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

void CameraPropertyControl::OnFloatPropertyChanged(PropertyList *forList, const String &forKey, float newValue)
{
    Camera *camera = dynamic_cast<Camera *> (currentNode);
    if("Fov" == forKey || "zNear" == forKey || "zFar" == forKey)
    {
        camera->Setup(
                      propertyList->GetFloatPropertyValue("Fov"),
                      320.0f / 480.0f,
                      propertyList->GetFloatPropertyValue("zNear"),
                      propertyList->GetFloatPropertyValue("zFar"),
                      propertyList->GetBoolPropertyValue("isOrtho"));
    }
    else if("position.x" == forKey || "position.y" == forKey || "position.z" == forKey)
    {
        camera->SetPosition(Vector3(
                                    propertyList->GetFloatPropertyValue("position.x"),
                                    propertyList->GetFloatPropertyValue("position.y"),
                                    propertyList->GetFloatPropertyValue("position.z")));
    }
    else if("target.x" == forKey || "target.y" == forKey || "target.z" == forKey)
    {
        camera->SetTarget(Vector3(
                                  propertyList->GetFloatPropertyValue("target.x"),
                                  propertyList->GetFloatPropertyValue("target.y"),
                                  propertyList->GetFloatPropertyValue("target.z")));
    }

    NodesPropertyControl::OnFloatPropertyChanged(forList, forKey, newValue);
}

void CameraPropertyControl::OnBoolPropertyChanged(PropertyList *forList, const String &forKey, bool newValue)
{
    if("isOrtho" == forKey)
    {
        Camera *camera = dynamic_cast<Camera *> (currentNode);
        camera->Setup(
                      propertyList->GetFloatPropertyValue("Fov"),
                      320.0f / 480.0f,
                      propertyList->GetFloatPropertyValue("zNear"),
                      propertyList->GetFloatPropertyValue("zFar"),
                      propertyList->GetBoolPropertyValue("isOrtho"));
    }

    NodesPropertyControl::OnBoolPropertyChanged(forList, forKey, newValue);
}
