#pragma once

#include "Base/BaseTypes.h"
#include "Entity/Component.h"
#include "Math/Matrix4.h"
#include "Math/Rect.h"
#include "Math/Vector.h"
#include "Reflection/Reflection.h"

namespace DAVA
{
class ScreenPositionComponent : public Component
{
    DAVA_VIRTUAL_REFLECTION(ScreenPositionComponent, Component);

public:
    IMPLEMENT_COMPONENT_TYPE(SCREEN_POSITION_COMPONENT);

    ScreenPositionComponent();
    ScreenPositionComponent(const ScreenPositionComponent& src);
    ScreenPositionComponent* Clone(Entity* toEntity) override;

    const Vector3& GetCameraPosition() const;
    void SetCameraPosition(const Vector3& v);
    const Vector3& GetCameraDirection() const;
    void SetCameraDirection(const Vector3& v);
    const Matrix4& GetCameraViewProjMatrix() const;
    void SetCameraViewProjMatrix(const Matrix4& m);
    const Rect& GetCameraViewport() const;
    void SetCameraViewport(const Rect& r);
    const Vector3& GetWorldPosition() const;
    void SetWorldPosition(const Vector3& v);
    const Vector3& GetScreenPositionAndDepth() const;
    void SetScreenPositionAndDepth(const Vector3& v);

protected:
    ~ScreenPositionComponent();

private:
    Vector3 cameraPosition;
    Vector3 cameraDirection;
    Matrix4 cameraViewProjMatrix;
    Rect cameraViewport;
    Vector3 worldPosition;
    Vector3 screenPositionAndDepth;
};

inline const Vector3& ScreenPositionComponent::GetCameraPosition() const
{
    return cameraPosition;
}

inline const Vector3& ScreenPositionComponent::GetCameraDirection() const
{
    return cameraDirection;
}

inline const Matrix4& ScreenPositionComponent::GetCameraViewProjMatrix() const
{
    return cameraViewProjMatrix;
}

inline const Rect& ScreenPositionComponent::GetCameraViewport() const
{
    return cameraViewport;
}

inline const Vector3& ScreenPositionComponent::GetWorldPosition() const
{
    return worldPosition;
}

inline const Vector3& ScreenPositionComponent::GetScreenPositionAndDepth() const
{
    return screenPositionAndDepth;
}

}
