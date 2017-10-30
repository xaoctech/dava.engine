#include "UI/Joypad/UIJoypadComponent.h"
#include "Reflection/ReflectionRegistrator.h"
#include "UI/UIControl.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(UIJoypadComponent)
{
    ReflectionRegistrator<UIJoypadComponent>::Begin()
        .ConstructorByValue()
        .ConstructorByPointer()
        .DestructorByPointer([](UIJoypadComponent* c) { SafeRelease(c); })
        .Field("stickArea", &UIJoypadComponent::GetStickArea, &UIJoypadComponent::SetStickArea)
        .Field("stickArm", &UIJoypadComponent::GetStickArm, &UIJoypadComponent::SetStickArm)
        .Field("stickArrow", &UIJoypadComponent::GetStickArrow, &UIJoypadComponent::SetStickArrow)
        .Field("stickAreaRadius", &UIJoypadComponent::GetStickAreaRadius, &UIJoypadComponent::SetStickAreaRadius)
        .Field("isActive", &UIJoypadComponent::IsActive, &UIJoypadComponent::SetActiveFlag)
        .Field("isDynamic", &UIJoypadComponent::IsDynamic, &UIJoypadComponent::SetDynamicFlag)
        .Field("coords", &UIJoypadComponent::GetOriginalCoords, &UIJoypadComponent::SetOriginalCoords)
        .Field("activationThreshold", &UIJoypadComponent::GetActivationThreshold, &UIJoypadComponent::SetActivationThreshold)
        .Field("initialPosition", &UIJoypadComponent::GetInitialPosition, &UIJoypadComponent::SetInitialPosition)
        .Field("cancelZone", &UIJoypadComponent::GetCancelZone, &UIJoypadComponent::SetCancelZone)
        .Field("cancelRadius", &UIJoypadComponent::GetCancelRadius, &UIJoypadComponent::SetCancelRadius)
        .Method("GetTransformedCoords", &UIJoypadComponent::GetTransformedCoords)
        .End();
}

UIJoypadComponent::UIJoypadComponent(const UIJoypadComponent& other)
    : UIComponent(other)
    , stickArrow(other.stickArrow)
    , stickArm(other.stickArm)
    , stickArea(other.stickArea)
    , coordsTransformFn(other.coordsTransformFn)
    , stickAreaRadius(other.stickAreaRadius)
    , isDynamic(other.isDynamic)
    , isActive(other.isActive)
    , coords(other.coords)
    , initialPosition(other.initialPosition)
    , activationThreshold(other.activationThreshold)
    , cancelZone(other.cancelZone)
    , cancelRadius(other.cancelRadius)
{
}

UIJoypadComponent* UIJoypadComponent::Clone() const
{
    return new UIJoypadComponent(*this);
}

bool UIJoypadComponent::IsDynamic() const
{
    return isDynamic;
}

void UIJoypadComponent::SetDynamicFlag(bool dynamic)
{
    isDynamic = dynamic;
}

UIControl* UIJoypadComponent::GetStickArea() const
{
    return stickArea;
}

void UIJoypadComponent::SetStickArea(UIControl* stickArea_)
{
    if (stickArea_ != nullptr)
    {
        stickArea_->SetPivot({ 0.5f, 0.5f });
        stickArea_->SetInputEnabled(false);
    }

    stickArea = stickArea_;
}

UIControl* UIJoypadComponent::GetStickArm() const
{
    return stickArm;
}

void UIJoypadComponent::SetStickArm(UIControl* stickArm_)
{
    if (stickArm_ != nullptr)
    {
        stickArm_->SetPivot({ 0.5f, 0.5f });
        stickArm_->SetInputEnabled(false);
    }

    stickArm = stickArm_;
}

UIControl* UIJoypadComponent::GetStickArrow() const
{
    return stickArrow;
}

void UIJoypadComponent::SetStickArrow(UIControl* stickArrow_)
{
    if (stickArrow_ != nullptr)
    {
        stickArrow_->SetPivot({ 0.5f, 0.5f });
        stickArrow_->SetInputEnabled(false);
    }

    stickArrow = stickArrow_;
}

void UIJoypadComponent::SetCoordsTransformFunction(CoordsTransformFn fn)
{
    coordsTransformFn = fn;
}

const UIJoypadComponent::CoordsTransformFn& UIJoypadComponent::GetCoordsTransformFunction()
{
    return coordsTransformFn;
}

float32 UIJoypadComponent::GetStickAreaRadius() const
{
    return stickAreaRadius;
}

void UIJoypadComponent::SetStickAreaRadius(float32 radius)
{
    stickAreaRadius = radius;
}

bool UIJoypadComponent::IsActive() const
{
    return isActive;
}

void UIJoypadComponent::SetActiveFlag(bool active)
{
    isActive = active;
}

Vector2 UIJoypadComponent::GetOriginalCoords() const
{
    return coords;
}

void UIJoypadComponent::SetOriginalCoords(Vector2 coords_)
{
    coords = coords_;
}

Vector2 UIJoypadComponent::GetInitialPosition() const
{
    return initialPosition;
}

void UIJoypadComponent::SetInitialPosition(Vector2 position)
{
    initialPosition = position;
}

float32 UIJoypadComponent::GetActivationThreshold() const
{
    return activationThreshold;
}

void UIJoypadComponent::SetActivationThreshold(float32 threshold)
{
    activationThreshold = threshold;
}

float32 UIJoypadComponent::GetCancelRadius() const
{
    return cancelRadius;
}

void UIJoypadComponent::SetCancelRadius(float32 radius)
{
    cancelRadius = radius;
}

const Rect& UIJoypadComponent::GetCancelZone() const
{
    return cancelZone;
}

void UIJoypadComponent::SetCancelZone(const Rect& zone)
{
    cancelZone = zone;
}

Vector2 UIJoypadComponent::GetTransformedCoords() const
{
    if (coordsTransformFn)
    {
        return coordsTransformFn(coords);
    }

    return coords;
}
}