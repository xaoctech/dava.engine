#include "Scene3D/Components/CameraComponent.h"
#include "Scene3D/Camera.h"
#include "Scene3D/SceneNode.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Systems/EventSystem.h"

namespace DAVA
{
    
CameraComponent::CameraComponent(Camera * _camera)
{
    camera = SafeRetain(_camera);
}

CameraComponent::~CameraComponent()
{
    SafeRelease(camera);
}
    
void CameraComponent::SetCamera(Camera * _camera)
{
    camera = SafeRetain(_camera);
}

    
Component * CameraComponent::Clone()
{
    CameraComponent * newComponent = new CameraComponent();
    //newComponent->camera = this->camera->Clone();

    return newComponent;
}
    
};