#include "Scene3D/Components/ScreenPositionComponent.h"
#include "Reflection/ReflectedMeta.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Scene3D/Entity.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(ScreenPositionComponent)
{
    ReflectionRegistrator<ScreenPositionComponent>::Begin()
    .ConstructorByPointer()
    .Field("cameraPosition", &ScreenPositionComponent::GetCameraPosition, &ScreenPositionComponent::SetCameraPosition)[M::ReadOnly(), M::DisplayName("Camera position")]
    .Field("cameraDirection", &ScreenPositionComponent::GetCameraDirection, &ScreenPositionComponent::SetCameraDirection)[M::ReadOnly(), M::DisplayName("Camera direction")]
    .Field("cameraViewProjMatrix", &ScreenPositionComponent::GetCameraViewProjMatrix, &ScreenPositionComponent::SetCameraViewProjMatrix)[M::ReadOnly(), M::DisplayName("Camera view-proj matrix")]
    .Field("cameraViewport", &ScreenPositionComponent::GetCameraViewport, &ScreenPositionComponent::SetCameraViewport)[M::ReadOnly(), M::DisplayName("Camera viewport")]
    .Field("worldPosition", &ScreenPositionComponent::GetWorldPosition, &ScreenPositionComponent::SetWorldPosition)[M::ReadOnly(), M::DisplayName("World position")]
    .Field("screenPositionAndDepth", &ScreenPositionComponent::GetScreenPositionAndDepth, &ScreenPositionComponent::SetScreenPositionAndDepth)[M::ReadOnly(), M::DisplayName("Screen position and depth")]
    .End();
}

ScreenPositionComponent::ScreenPositionComponent() = default;

ScreenPositionComponent::~ScreenPositionComponent() = default;

ScreenPositionComponent::ScreenPositionComponent(const ScreenPositionComponent& src) = default;

ScreenPositionComponent* ScreenPositionComponent::Clone(Entity* toEntity)
{
    ScreenPositionComponent* uc = new ScreenPositionComponent(*this);
    uc->SetEntity(toEntity);
    return uc;
}

void ScreenPositionComponent::SetCameraPosition(const Vector3& v)
{
    cameraPosition = v;
}

void ScreenPositionComponent::SetCameraDirection(const Vector3& v)
{
    cameraDirection = v;
}

void ScreenPositionComponent::SetCameraViewProjMatrix(const Matrix4& m)
{
    cameraViewProjMatrix = m;
}

void ScreenPositionComponent::SetCameraViewport(const Rect& r)
{
    cameraViewport = r;
}

void ScreenPositionComponent::SetWorldPosition(const Vector3& v)
{
    worldPosition = v;
}

void ScreenPositionComponent::SetScreenPositionAndDepth(const Vector3& v)
{
    screenPositionAndDepth = v;
}

}
