#include "Scene3D/Systems/ScreenPositionSystem.h"
#include "Scene3D/Components/ScreenPositionComponent.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"
#include "UI/UI3DView.h"

namespace DAVA
{
ScreenPositionSystem::ScreenPositionSystem(Scene* scene)
    : SceneSystem(scene)
{
}

void ScreenPositionSystem::AddEntity(Entity* entity)
{
    ScreenPositionComponent* component = static_cast<ScreenPositionComponent*>(entity->GetComponent(Component::SCREEN_POSITION_COMPONENT));
    if (component)
    {
        components.push_back(component);
    }
}

void ScreenPositionSystem::RemoveEntity(Entity* entity)
{
    ScreenPositionComponent* component = static_cast<ScreenPositionComponent*>(entity->GetComponent(Component::SCREEN_POSITION_COMPONENT));
    if (component)
    {
        components.erase(std::remove(components.begin(), components.end(), component), components.end());
    }
}

void ScreenPositionSystem::PrepareForRemove()
{
    components.clear();
}

void ScreenPositionSystem::Process(float32 timeElapsed)
{
    if (components.empty())
    {
        return;
    }

    Vector3 cameraPosition;
    Vector3 cameraDirection;
    Matrix4 cameraViewProjMatrix;
    Rect cameraViewport;
    Camera* camera = GetScene()->GetCurrentCamera();
    if (camera)
    {
        cameraPosition = camera->GetPosition();
        cameraDirection = camera->GetDirection();
        cameraViewProjMatrix = camera->GetViewProjMatrix();
    }
    if (ui3dView)
    {
        cameraViewport = ui3dView->GetAbsoluteRect();
    }

    for (ScreenPositionComponent* component : components)
    {
        Vector3 worldPosition = component->GetEntity()->GetWorldTransform().GetTranslationVector();

        component->SetCameraPosition(cameraPosition);
        component->SetCameraDirection(cameraDirection);
        component->SetCameraViewProjMatrix(cameraViewProjMatrix);
        component->SetCameraViewport(cameraViewport);
        component->SetWorldPosition(worldPosition);

        // TODO: Need frustum test? camera->GetFrustum()->IsInside(worldPosition, 1.0f);

        if (camera && cameraViewport.dx > 0.f && cameraViewport.dy > 0.f)
        {
            if (FLOAT_EQUAL(worldPosition.z, camera->GetPosition().z)) // Check entity and camera Z position
            {
                worldPosition.z += DAVA::EPSILON; // Fix Z position
            }
            component->SetScreenPositionAndDepth(camera->GetOnScreenPositionAndDepth(worldPosition, cameraViewport));
        }
        else
        {
            component->SetScreenPositionAndDepth(Vector3::Zero);
        }
    }
}

void ScreenPositionSystem::SetUI3DView(UI3DView* view)
{
    ui3dView = view;
}

UI3DView* ScreenPositionSystem::GetUI3DView() const
{
    return ui3dView;
}
}