#pragma once

#include "Base/RefPtr.h"
#include "Functional/Function.h"
#include "Math/Vector.h"
#include "Reflection/Reflection.h"
#include "UI/Components/UIComponent.h"

namespace DAVA
{
class Entity;
class ScreenPositionComponent;

class UIEntityMarkerComponent : public UIComponent
{
    DAVA_VIRTUAL_REFLECTION(UIEntityMarkerComponent, UIComponent);
    DECLARE_UI_COMPONENT(UIEntityMarkerComponent);

public:
    enum class OrderMode : int32
    {
        NearFront = 0,
        NearBack
    };

    using CustomStrategy = Function<void(UIControl*, UIEntityMarkerComponent*, ScreenPositionComponent*)>;

    UIEntityMarkerComponent();
    UIEntityMarkerComponent(const UIEntityMarkerComponent& src);
    UIEntityMarkerComponent& operator=(const UIEntityMarkerComponent&) = delete;

    UIEntityMarkerComponent* Clone() const override;

    Entity* GetTargetEntity() const;
    void SetTargetEntity(Entity* e);

    bool IsEnabled() const;
    void SetEnabled(bool enable);

    bool IsSyncVisibilityEnabled() const;
    void SetSyncVisibilityEnabled(bool enable);

    bool IsSyncPositionEnabled() const;
    void SetSyncPositionEnabled(bool enable);

    bool IsSyncScaleEnabled() const;
    void SetSyncScaleEnabled(bool enable);
    const Vector2& GetScaleFactor() const;
    void SetScaleFactor(const Vector2& factor);
    const Vector2& GetMaxScale() const;
    void SetMaxScale(const Vector2& s);
    const Vector2& GetMinScale() const;
    void SetMinScale(const Vector2& s);

    bool IsSyncOrderEnabled() const;
    void SetSyncOrderEnabled(bool enable);
    OrderMode GetOrderMode() const;
    void SetOrderMode(OrderMode mode);

    bool IsUseCustomStrategy() const;
    void SetUseCustomStrategy(bool enable);
    const CustomStrategy& GetCustomStrategy() const;
    void SetCustomStrategy(const CustomStrategy& fn);

protected:
    ~UIEntityMarkerComponent();

private:
    RefPtr<Entity> targetEntity;
    bool enabled = true;
    // Visibility
    bool syncVisibilityEnabled = false;
    // Position
    bool syncPositionEnabled = false;
    // Scale
    bool syncScaleEnabled = false;
    Vector2 scaleFactor = Vector2(1.f, 1.f);
    Vector2 maxScale = Vector2(2.f, 2.f);
    Vector2 minScale = Vector2(.1f, .1f);
    // Order
    bool syncOrderEnabled = false;
    OrderMode orderMode = OrderMode::NearFront;
    // Custom strategy
    bool useCustomStrategy = false;
    CustomStrategy customStrategy;
};

inline Entity* UIEntityMarkerComponent::GetTargetEntity() const
{
    return targetEntity.Get();
}

inline void UIEntityMarkerComponent::SetTargetEntity(Entity* e)
{
    targetEntity = e;
}

inline bool UIEntityMarkerComponent::IsEnabled() const
{
    return enabled;
}

inline void UIEntityMarkerComponent::SetEnabled(bool enable)
{
    enabled = enable;
}

inline bool UIEntityMarkerComponent::IsSyncVisibilityEnabled() const
{
    return syncVisibilityEnabled;
}

inline void UIEntityMarkerComponent::SetSyncVisibilityEnabled(bool absolute)
{
    syncVisibilityEnabled = absolute;
}

inline bool UIEntityMarkerComponent::IsSyncPositionEnabled() const
{
    return syncPositionEnabled;
}

inline void UIEntityMarkerComponent::SetSyncPositionEnabled(bool absolute)
{
    syncPositionEnabled = absolute;
}

inline bool UIEntityMarkerComponent::IsSyncScaleEnabled() const
{
    return syncScaleEnabled;
}

inline void UIEntityMarkerComponent::SetSyncScaleEnabled(bool absolute)
{
    syncScaleEnabled = absolute;
}

inline const Vector2& UIEntityMarkerComponent::GetScaleFactor() const
{
    return scaleFactor;
}

inline void UIEntityMarkerComponent::SetScaleFactor(const Vector2& factor)
{
    scaleFactor = factor;
}

inline const Vector2& UIEntityMarkerComponent::GetMaxScale() const
{
    return maxScale;
}

inline void UIEntityMarkerComponent::SetMaxScale(const Vector2& s)
{
    maxScale = s;
}

inline const Vector2& UIEntityMarkerComponent::GetMinScale() const
{
    return minScale;
}

inline void UIEntityMarkerComponent::SetMinScale(const Vector2& s)
{
    minScale = s;
}

inline bool UIEntityMarkerComponent::IsSyncOrderEnabled() const
{
    return syncOrderEnabled;
}

inline void UIEntityMarkerComponent::SetSyncOrderEnabled(bool enable)
{
    syncOrderEnabled = enable;
}

inline UIEntityMarkerComponent::OrderMode UIEntityMarkerComponent::GetOrderMode() const
{
    return orderMode;
}

inline void UIEntityMarkerComponent::SetOrderMode(OrderMode mode)
{
    orderMode = mode;
}

inline bool UIEntityMarkerComponent::IsUseCustomStrategy() const
{
    return useCustomStrategy;
}   

inline void UIEntityMarkerComponent::SetUseCustomStrategy(bool enable)
{
    useCustomStrategy = enable;
}

inline const UIEntityMarkerComponent::CustomStrategy& UIEntityMarkerComponent::GetCustomStrategy() const
{
    return customStrategy;
}

void UIEntityMarkerComponent::SetCustomStrategy(const CustomStrategy& fn)
{
    customStrategy = fn;
}

}
