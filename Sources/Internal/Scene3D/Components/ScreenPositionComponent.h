#pragma once

#include "Base/BaseTypes.h"
#include "Entity/Component.h"
#include "Math/Matrix4.h"
#include "Math/Rect.h"
#include "Math/Vector.h"
#include "Reflection/Reflection.h"

namespace DAVA
{

/** Component which contains data of Camera and screen position of Entity. */
class ScreenPositionComponent : public Component
{
    DAVA_VIRTUAL_REFLECTION(ScreenPositionComponent, Component);

public:
    IMPLEMENT_COMPONENT_TYPE(SCREEN_POSITION_COMPONENT);

    ScreenPositionComponent();
    ScreenPositionComponent(const ScreenPositionComponent& src);
    ScreenPositionComponent* Clone(Entity* toEntity) override;

    /** Return stored camera position. */
    const Vector3& GetCameraPosition() const;
    /** Store camera position. */
    void SetCameraPosition(const Vector3& v);
    /** Return stored camera direction. */
    const Vector3& GetCameraDirection() const;
    /** Store camera direction. */
    void SetCameraDirection(const Vector3& v);
    /** Return stored camera view projection matrix. */
    const Matrix4& GetCameraViewProjMatrix() const;
    /** Store camera view projection matrix. */
    void SetCameraViewProjMatrix(const Matrix4& m);
    /** Return stored camera viewport rect. */
    const Rect& GetCameraViewport() const;
    /** Store camera viewport rect. */
    void SetCameraViewport(const Rect& r);
    /** Return stored entity world position. */
    const Vector3& GetWorldPosition() const;
    /** Store entity world position. */
    void SetWorldPosition(const Vector3& v);
    /** Return stored entity screen position and depth. */
    const Vector3& GetScreenPositionAndDepth() const;
    /** Store entity screen position and depth. */
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
